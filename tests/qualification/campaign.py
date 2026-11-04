#                              _       _
#                           __| | ___ | |__   __ _
#                          / _` |/ _ \| '_ \ / _` |
#                         | (_| | (_) | |_) | (_| |
#                          \__,_|\___/|_.__/ \__,_|
#
#                              Apache License
#                        Version 2.0, January 2004
#                     http://www.apache.org/licenses/LICENSE-2.0
#
# Copyright 2026 martianLabs
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import re
import subprocess
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'helpers'))
from sanitizer_diagnostics import require_clean_diagnostics


# +---------------------------------------------------------------------------+
# | [>] required_groups                                          ( function ) |
# +---------------------------------------------------------------------------+
def required_groups(windows, tls):
  groups = ['doba_security_model_tests', 'doba_engine_concurrency_tests',
            'doba_stream_integrity_tests', 'doba_test_diagnostics_tests',
            'doba_qualification_tests']
  native = (['iocp_completion', 'windows_lifecycle', 'windows_processing',
             'network_fault']
            if windows else ['linux_transport', 'linux_accept_error',
                             'linux_processing'])
  groups.extend('doba_' + name + '_tests' for name in native)
  if tls:
    groups.extend('doba_tls_' + name + '_tests' for name in
                  ('core', 'processing', 'processing_wire'))
    groups.append('doba_tls_tests')
  return groups


# +---------------------------------------------------------------------------+
# | [>] validate_inventory                                       ( function ) |
# +---------------------------------------------------------------------------+
def validate_inventory(inventory, required):
  tests = {item['name']: item for item in inventory['tests']}
  if len(tests) != len(inventory['tests']):
    raise RuntimeError('Duplicate registered test name')
  for name in required:
    if name not in tests or not tests[name].get('command'):
      raise RuntimeError('Missing executable campaign group: ' + name)
  return tests


# +---------------------------------------------------------------------------+
# | [>] case_names                                               ( function ) |
# +---------------------------------------------------------------------------+
def case_names(listing):
  return [match.group(1) for line in listing.splitlines()
          if (match := re.match(r'^.+:\d+ - (.+)$', line))]


# +---------------------------------------------------------------------------+
# | [>] execute                                                  ( function ) |
# +---------------------------------------------------------------------------+
def execute(command, log, timeout, expected=None):
  try:
    result = subprocess.run(command, capture_output=True, timeout=timeout)
  except subprocess.TimeoutExpired as error:
    log.write_bytes((error.stdout or b'') + (error.stderr or b'') +
                    b'\nCampaign subprocess deadline exceeded\n')
    raise
  text = (result.stdout + result.stderr).decode('utf-8', errors='replace')
  log.write_text(text, encoding='utf-8')
  require_clean_diagnostics(text)
  if result.returncode or 'assertion failed:' in text:
    raise RuntimeError(f'Campaign failure ({result.returncode}): {log}')
  if expected and not re.search(expected, text):
    raise RuntimeError(f'Campaign selection not proven: {log}')
  return text


# +---------------------------------------------------------------------------+
# | [>] plan                                                     ( function ) |
# +---------------------------------------------------------------------------+
def plan(build, output, tls):
  windows = os.name == 'nt'
  suffix = '.exe' if windows else ''
  raw = subprocess.check_output(
      ['ctest', '--test-dir', str(build), '--show-only=json-v1'], timeout=30)
  inventory = json.loads(raw)
  groups = required_groups(windows, tls)
  validate_inventory(inventory, groups)
  (output / 'ctest-inventory.json').write_bytes(raw)
  actions = []
  for name in groups:
    actions.append({'id': name, 'command':
        ['ctest', '--test-dir', str(build), '-R', '^' + name + '$',
         '--output-on-failure', '--no-tests=error'],
        'selection': (r'1/1 Test\s+#\d+: ' + re.escape(name) +
                      r'\b[^\r\n]*\bPassed\b')})
  files = [('common/cache_tests.cpp', 13),
           ('common/byte_storage_tests.cpp', 19),
           ('common/reader_tests.cpp', 9),
           ('protocol/http/v11/operating_limits_tests.cpp', 1)]
  if not windows:
    files.append(('transport/server/tcp_context_linux_tests.cpp', 17))
  unit = str(build / ('tests/unit/doba_unit_tests' + suffix))
  for index, (file, minimum) in enumerate(files):
    listing = execute([unit, '--file', file, '--list'],
                       output / f'unit-{index}-inventory.log', 30)
    names = case_names(listing)
    if len(names) < minimum or len(names) != len(set(names)):
      raise RuntimeError('Missing or duplicated fault cases: ' + file)
    # Every discovered case runs in its own process, making order and failure
    # evidence explicit; the native probes control their own interleavings.
    for name in names:
      actions.append({'id': file + ':' + name,
                      'command': [unit, '--file', file, '--name', name],
                      'selection': r'running 1 unit tests'})
  return actions


# +---------------------------------------------------------------------------+
# | [>] main                                                     ( function ) |
# +---------------------------------------------------------------------------+
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--build', type=Path, required=True)
  parser.add_argument('--output', type=Path, required=True)
  parser.add_argument('--rounds', type=int, default=2)
  parser.add_argument('--seed', type=int, default=104729)
  parser.add_argument('--tls', action='store_true')
  args = parser.parse_args()
  if not 1 <= args.rounds <= 32 or args.output.exists():
    parser.error('1..32 rounds and a fresh output directory required')
  build, output = args.build.resolve(), args.output.resolve()
  output.mkdir(parents=True)
  cache = (build / 'CMakeCache.txt').read_text(encoding='utf-8')
  if not re.search(r'^CMAKE_BUILD_TYPE:STRING=Release$', cache, re.MULTILINE):
    raise RuntimeError('Campaigns require Release builds')
  root = Path(__file__).resolve().parents[2]
  hashes = {p.relative_to(root).as_posix():
            hashlib.sha256(p.read_bytes()).hexdigest()
            for directory in ('include', 'tests')
            for p in (root / directory).rglob('*')
            if p.is_file() and '__pycache__' not in p.parts}
  (output / 'source-sha256.json').write_text(json.dumps(hashes, indent=2))
  state = {'status': 'failed', 'platform': platform.platform(),
           'ci_revision': os.environ.get('GITHUB_SHA'),
           'seed': args.seed, 'rounds': args.rounds,
           'sanitizer_environment': {key: os.environ.get(key) for key in
              ('ASAN_OPTIONS', 'UBSAN_OPTIONS', 'TSAN_OPTIONS')},
           'cases': []}
  try:
    actions = plan(build, output, args.tls)
    state['actions_per_round'] = len(actions)
    for round_id in range(args.rounds):
      order = actions.copy()
      random.Random(args.seed + round_id).shuffle(order)
      for index, action in enumerate(order):
        record = dict(action, round=round_id, status='failed')
        state['cases'].append(record)
        execute(action['command'],
                output / f'round-{round_id:02}-case-{index:03}.log',
                timeout=180, expected=action['selection'])
        record['status'] = 'passed'
    state['status'] = 'passed'
  finally:
    (output / 'results.json').write_text(json.dumps(state, indent=2) + '\n')
  print(f"Campaign passed: {len(state['cases'])} actions")


if __name__ == '__main__':
  main()
