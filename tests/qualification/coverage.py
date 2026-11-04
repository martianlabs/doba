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
import math
import os
from pathlib import Path
import platform
import subprocess
import sys

sys.dont_write_bytecode = True
from coverage_union import merge, summarize, html_report, markdown_report


# +---------------------------------------------------------------------------+
# | [>] write_json                                               ( function ) |
# +---------------------------------------------------------------------------+
def write_json(path, value):
  path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


# +---------------------------------------------------------------------------+
# | [>] run                                                      ( function ) |
# +---------------------------------------------------------------------------+
def run(command, output, name, env=None, check=True, timeout=1800):
  write_json(output / (name + '-command.json'), command)
  with (output / (name + '.log')).open('wb') as log:
    result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                            env=env, timeout=timeout)
  if check and result.returncode:
    raise RuntimeError(f'{name} failed: see {output / (name + ".log")}')
  return result.returncode


# +---------------------------------------------------------------------------+
# | [>] llvm                                                     ( function ) |
# +---------------------------------------------------------------------------+
def llvm(tool, arguments, output, name, destination=None):
  # Response files avoid Windows' command length limit; forward slashes keep
  # LLVM's response-file escaping independent from the host shell.
  response = output / (name + '.rsp')
  response.write_text('\n'.join(json.dumps(str(a).replace('\\', '/'),
                                         ensure_ascii=False)
                                for a in arguments[1:]), encoding='utf-8')
  command = [tool, arguments[0], '@' + str(response)]
  if destination is None:
    run(command, output, name)
  else:
    with destination.open('wb') as data, \
         (output / (name + '.log')).open('wb') as errors:
      result = subprocess.run(command, stdout=data,
                              stderr=errors, timeout=300)
    if result.returncode:
      raise RuntimeError(f'{name} failed: {errors.name}')


# +---------------------------------------------------------------------------+
# | [>] module                                                   ( function ) |
# +---------------------------------------------------------------------------+
def module(name):
  if name.startswith('include/common/'):
    return 'common'
  if name.startswith('include/network/'):
    return 'network'
  if name.startswith('include/transport/'):
    return 'tls' if '/tls' in name else 'tcp'
  if name.startswith('include/protocol/http/v11/'):
    part = name.removeprefix('include/protocol/http/v11/')
    if part.startswith(('body/', 'headers/', 'decoder')):
      return 'http-parser'
    if part.startswith(('engine', 'request_processor', 'pending_response',
                        'connection', 'context')):
      return 'http-engine'
    if part.startswith('static_file'):
      return 'static-files'
    return 'http-messages'
  return 'http-common' if '/http/' in name else 'public-contracts'


# +---------------------------------------------------------------------------+
# | [>] enforce                                                  ( function ) |
# +---------------------------------------------------------------------------+
def enforce(summary, baseline):
  errors = []
  for name in set(summary['modules']) - set(baseline['modules']):
    errors.append('Unreviewed measured module: ' + name)
  for name, minimum in baseline['modules'].items():
    current = summary['modules'].get(name)
    if current is None:
      errors.append(f'Missing measured module: {name}')
      continue
    for metric in ('lines', 'branches'):
      if minimum[metric] is None:
        if current[metric]['count']:
          errors.append(f'Missing coverage floor: {name}/{metric}')
        continue
      floor = minimum[metric]
      if (type(floor) not in (int, float) or not math.isfinite(floor) or
          not 0 <= floor <= 100):
        errors.append(f'Invalid coverage floor: {name}/{metric}')
        continue
      value = current[metric]['percent']
      if value is None or value + 1e-9 < minimum[metric]:
        errors.append(f'{name}/{metric}: {value} < {minimum[metric]}')
  missing = set(baseline['measured_headers']) - set(summary['measured_headers'])
  errors.extend('Lost measured header: ' + name for name in sorted(missing))
  unknown = (set(summary['unmapped_headers']) -
             set(baseline['unmapped_headers']))
  errors.extend('New unmapped header: ' + name for name in sorted(unknown))
  if errors:
    raise RuntimeError('Coverage regression:\n' + '\n'.join(errors))


# +---------------------------------------------------------------------------+
# | [>] main                                                     ( function ) |
# +---------------------------------------------------------------------------+
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--build', required=True, type=Path)
  parser.add_argument('--output', required=True, type=Path)
  parser.add_argument('--compiler', default='clang++')
  parser.add_argument('--llvm-cov', default='llvm-cov')
  parser.add_argument('--llvm-profdata', default='llvm-profdata')
  parser.add_argument('--openssl-root')
  parser.add_argument('--baseline', type=Path)
  parser.add_argument('--parallel', type=int, default=2)
  args = parser.parse_args()
  root = Path(__file__).resolve().parents[2]
  build, output = args.build.resolve(), args.output.resolve()
  if args.parallel < 1 or output.exists():
    parser.error('positive parallelism and a fresh output directory required')
  output.mkdir(parents=True)
  status = {'status': 'failed', 'platform': platform.platform(),
            'release_only': True, 'root': str(root), 'build': str(build)}
  try:
    query = build / '.cmake/api/v1/query'
    query.mkdir(parents=True, exist_ok=True)
    (query / 'codemodel-v2').touch()
    flags = ('-fprofile-instr-generate -fcoverage-mapping '
             '-fprofile-update=atomic -include "' +
             (root / 'tests/qualification/profile_identity.h').as_posix() + '"')
    command = ['cmake', '-S', str(root), '-B', str(build), '-G', 'Ninja',
               '-DCMAKE_BUILD_TYPE=Release',
               '-DCMAKE_CXX_COMPILER=' + args.compiler,
               '-DCMAKE_CXX_FLAGS=' + flags,
               '-DCMAKE_EXE_LINKER_FLAGS=-fprofile-instr-generate',
               '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
               '-DCMAKE_PROJECT_INCLUDE=' + str(
                   root / 'tests/qualification/instrument.cmake'),
               '-DPython3_EXECUTABLE=' + sys.executable,
               '-DDOBA_ENABLE_STRICT_WARNINGS=ON', '-DDOBA_ENABLE_TLS=ON',
               '-DDOBA_BUILD_TESTS=ON', '-DDOBA_BUILD_EXAMPLES=ON']
    if args.openssl_root:
      command.append('-DOPENSSL_ROOT_DIR=' + args.openssl_root)
    run(command, output, 'configure')
    run(['cmake', '--build', str(build), '--parallel', str(args.parallel)],
        output, 'build')
    for tool, name in ((args.compiler, 'compiler'),
                       (args.llvm_cov, 'llvm-cov'),
                       (args.llvm_profdata, 'llvm-profdata')):
      run([tool, '--version'], output, name)
    hashes = {p.relative_to(root).as_posix():
              hashlib.sha256(p.read_bytes()).hexdigest()
              for directory in ('include', 'tests', 'cmake')
              for p in (root / directory).rglob('*')
              if p.is_file() and '__pycache__' not in p.parts}
    write_json(output / 'source-sha256.json', hashes)
    env = dict(os.environ, LLVM_PROFILE_FILE=
               str(output / 'profiles/%p-%m.profraw'))
    env.pop('DOBA_COVERAGE_IDENTIFY', None)
    status['ctest_exit'] = run(
        ['ctest', '--test-dir', str(build), '--output-on-failure',
         '--no-tests=error'], output, 'ctest', env=env, check=False)
    profiles = sorted((output / 'profiles').glob('*.profraw'))
    if not profiles:
      raise RuntimeError('No raw profiles produced')
    status['profiles'] = len(profiles)
    reply = build / '.cmake/api/v1/reply'
    index = json.loads(sorted(reply.glob('index-*.json'))[-1].read_text())
    model = json.loads((reply / index['reply']['codemodel-v2']['jsonFile'])
                       .read_text())
    objects = []
    for target in model['configurations'][0]['targets']:
      data = json.loads((reply / target['jsonFile']).read_text())
      if data['type'] == 'EXECUTABLE':
        objects.extend({'binary': str(build / a['path']),
                        'target': target['name']} for a in data['artifacts']
                       if os.name != 'nt' or a['path'].endswith('.exe'))
    if not objects:
      raise RuntimeError('No instrumented executable inventory')
    write_json(output / 'objects.json', objects)
    files, identities, matched = {}, [], set()
    for index, object in enumerate(objects):
      binary, target = object['binary'], object['target']
      directory = output / f'object-{index:03}'
      directory.mkdir()
      probe = dict(os.environ, DOBA_COVERAGE_IDENTIFY='1',
                   LLVM_PROFILE_FILE=str(directory / 'identity/%p-%m.profraw'))
      run([binary], directory, 'identify', env=probe, timeout=10)
      identity = list((directory / 'identity').glob('*.profraw'))
      if len(identity) != 1:
        raise RuntimeError('Expected one executable profile identity')
      prefix = target + '-'
      signature = identity[0].name[len(prefix):].split('-', 1)[1]
      if not identity[0].name.startswith(prefix):
        raise RuntimeError('Executable identity is not namespaced by target')
      if any(item['target'] == target for item in identities):
        raise RuntimeError('Executable profile namespace collision')
      own = [p for p in profiles if p.name.startswith(prefix) and
             p.name[len(prefix):].split('-', 1)[1] == signature]
      matched.update(own)
      identities.append({'binary': binary, 'signature': signature,
                         'target': target, 'observed_profiles': len(own)})
      merged = directory / 'coverage.profdata'
      llvm(args.llvm_profdata,
           ['merge', '-sparse', *(own or identity), '-o', merged],
           directory, 'merge')
      common = [binary, '-instr-profile=' + str(merged)]
      llvm(args.llvm_cov, ['export', *common, '-skip-functions'],
           directory, 'export', directory / 'coverage.json')
      llvm(args.llvm_cov, ['export', *common, '-format=lcov'],
           directory, 'lcov', directory / 'coverage.lcov')
      for name in ('export.log', 'lcov.log'):
        if (directory / name).read_text().strip():
          raise RuntimeError('Unqualified LLVM coverage diagnostic: ' + binary)
      merge(files, json.loads((directory / 'coverage.json').read_text()),
            (directory / 'coverage.lcov').read_text(), root)
    if matched != set(profiles):
      raise RuntimeError('Raw profiles have no matching executable identity')
    write_json(output / 'profile-identities.json', identities)
    summary = summarize(files, root, module)
    write_json(output / 'summary.json', summary)
    report = markdown_report(summary)
    (output / 'summary.md').write_text(report, encoding='utf-8')
    if os.environ.get('GITHUB_STEP_SUMMARY'):
      with Path(os.environ['GITHUB_STEP_SUMMARY']).open('a',
                                                      encoding='utf-8') as log:
        log.write(report)
    write_json(output / 'coverage.json', {
        name: {'lines': file['lines'], 'branches': [
            {'position': list(key), 'outcomes': value}
            for key, value in sorted(file['branches'].items())]}
        for name, file in sorted(files.items())})
    html_report(files, summary, root, output / 'html')
    if args.baseline:
      baseline = json.loads(args.baseline.read_text())
      expected = 'windows' if os.name == 'nt' else 'linux'
      if baseline.get('platform') != expected:
        raise RuntimeError('Coverage baseline platform does not match')
      enforce(summary, baseline)
    if status['ctest_exit']:
      raise RuntimeError('CTest failed; coverage is diagnostic only')
    status['status'] = 'passed'
  finally:
    write_json(output / 'status.json', status)
  print(json.dumps({'status': status['status'],
                    'modules': summary['modules']}, indent=2))


if __name__ == '__main__':
  main()
