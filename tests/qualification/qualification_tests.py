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

import copy
from pathlib import Path
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True
import coverage
import coverage_union
import campaign


# +---------------------------------------------------------------------------+
# | [>] rejected                                                 ( function ) |
# +---------------------------------------------------------------------------+
def rejected(function, diagnostic):
  try:
    function()
  except (RuntimeError, AssertionError) as error:
    if diagnostic not in str(error):
      raise AssertionError('Wrong failure diagnostic: ' + str(error))
    return
  raise AssertionError('Negative control accepted: ' + diagnostic)


# +---------------------------------------------------------------------------+
# | [>] coverage_controls                                        ( function ) |
# +---------------------------------------------------------------------------+
def coverage_controls(root):
  include = root / 'include/common'
  include.mkdir(parents=True)
  measured = include / 'measured.h'
  measured.write_text('<script>untrusted source</script>\n')
  (include / 'uninstantiated.h').touch()
  item = {'filename': str(measured), 'summary': {
      'lines': {'count': 3, 'covered': 2, 'percent': 100},
      'branches': {'count': 2, 'covered': 1, 'percent': 100}},
      'branches': [[2, 1, 2, 4, 0, 1, 0, 0, 0]]}
  document = {'data': [{'files': [item, {
      'filename': str(root / 'tests/test.cpp')}]}]}
  files = {}
  lcov = f'SF:{measured}\nDA:1,2\nDA:2,0\nDA:3,1\nend_of_record\n'
  coverage_union.merge(files, document, lcov, root)
  summary = coverage_union.summarize(files, root, coverage.module)
  stats = summary['modules']['common']
  if stats['lines']['percent'] != 100 * 2 / 3:
    raise AssertionError('Aggregate lines trusted supplied percentages')
  if stats['branches']['percent'] != 50:
    raise AssertionError('Branch outcomes were not measured separately')
  if summary['unmapped_headers'] != ['include/common/uninstantiated.h']:
    raise AssertionError('Uninstantiated header disappeared from inventory')
  if summary['uncovered_branch_outcomes'][0]['true'] != 0:
    raise AssertionError('Uncovered branch frontier was not reported')
  baseline = {'modules': {'common': {'lines': 66, 'branches': 50}},
              'measured_headers': ['include/common/measured.h'],
              'unmapped_headers': ['include/common/uninstantiated.h']}
  coverage.enforce(summary, baseline)
  for floor, message in ((float('nan'), 'Invalid coverage floor'),
                          (-1, 'Invalid coverage floor'),
                          (None, 'Missing coverage floor')):
    invalid = copy.deepcopy(baseline)
    invalid['modules']['common']['branches'] = floor
    rejected(lambda: coverage.enforce(summary, invalid), message)
  bad = copy.deepcopy(summary)
  bad['modules']['common']['branches']['percent'] = 49
  rejected(lambda: coverage.enforce(bad, baseline), 'common/branches')
  bad = copy.deepcopy(summary)
  bad['measured_headers'] = []
  rejected(lambda: coverage.enforce(bad, baseline), 'Lost measured header')
  bad = copy.deepcopy(summary)
  bad['unmapped_headers'].append('include/common/new_untested.h')
  rejected(lambda: coverage.enforce(bad, baseline), 'New unmapped header')
  bad = copy.deepcopy(summary)
  bad['modules'] = {}
  rejected(lambda: coverage.enforce(bad, baseline), 'Missing measured module')
  rejected(lambda: coverage_union.summarize({}, root, coverage.module),
           'No production coverage')
  second = copy.deepcopy(document)
  second['data'][0]['files'][0]['branches'][0][4:6] = [1, 0]
  more = f'SF:{measured}\nDA:2,1\nend_of_record\n'
  forward, reverse = {}, {}
  coverage_union.merge(forward, document, lcov, root)
  coverage_union.merge(forward, second, more, root)
  coverage_union.merge(reverse, second, more, root)
  coverage_union.merge(reverse, document, lcov, root)
  if forward != reverse:
    raise AssertionError('Coverage union depends on executable order')
  total = coverage_union.summarize(forward, root, coverage.module)
  if total['modules']['common']['branches']['percent'] != 100:
    raise AssertionError('Complementary branch outcomes were not combined')
  if total['modules']['common']['lines']['percent'] != 100:
    raise AssertionError('Executable lines were averaged or double counted')
  coverage_union.html_report(files, summary, root, root / 'html')
  html = (root / 'html/file-0.html').read_text()
  if '<script>' in html or '&lt;script&gt;' not in html:
    raise AssertionError('HTML report did not escape source text')


# +---------------------------------------------------------------------------+
# | [>] path_controls                                            ( function ) |
# +---------------------------------------------------------------------------+
def path_controls(root):
  source = root / 'source tree'
  header = source / 'include/common/probe.h'
  header.parent.mkdir(parents=True)
  header.touch()
  outside = root / 'neighbor/include/common/outside.h'
  outside.parent.mkdir(parents=True)
  outside.touch()
  aliases = [source, source / 'include/..']
  if sys.platform == 'win32':
    import ctypes
    from ctypes import wintypes
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    short_path = kernel.GetShortPathNameW
    short_path.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, wintypes.DWORD]
    short_path.restype = wintypes.DWORD
    buffer = ctypes.create_unicode_buffer(32768)
    size = short_path(str(source), buffer, len(buffer))
    if not size or size >= len(buffer):
      raise ctypes.WinError(ctypes.get_last_error())
    # Windows may preserve long names when 8.3 names are disabled. The dot
    # alias above always exercises noncanonical roots on both platforms.
    aliases.append(Path(buffer.value))
  for index, alias in enumerate(aliases):
    for filename in (str(header), header.as_posix()):
      excluded = source / 'tests/probe.h'
      sources = [filename, str(outside), str(excluded)]
      document = {'data': [{'files': [{'filename': name}
                                     for name in sources]}]}
      lcov = ''.join(f'SF:{name}\nDA:1,1\nend_of_record\n'
                     for name in sources)
      files = {}
      coverage_union.merge(files, document, lcov, alias)
      if set(files) != {'include/common/probe.h'}:
        raise AssertionError('Canonical containment lost or admitted files')
      summary = coverage_union.summarize(files, alias, coverage.module)
      if summary['modules']['common']['lines'] != \
          {'count': 1, 'covered': 1, 'percent': 100.0}:
        raise AssertionError('JSON/LCOV paths lost their production mapping')
    # Replay the complete failing CI fixture, including inventory and HTML,
    # with an unresolved root instead of normalizing the test input away.
    fixture = alias / f'full fixture {index}'
    fixture.mkdir()
    coverage_controls(fixture)


# +---------------------------------------------------------------------------+
# | [>] campaign_controls                                        ( function ) |
# +---------------------------------------------------------------------------+
def campaign_controls(root):
  if campaign.case_names('a.cpp:12 - first\r\nb.cpp:7 - second\r\n') != \
      ['first', 'second']:
    raise AssertionError('Windows line endings changed exact test names')
  required = campaign.required_groups(False, True)
  inventory = {'tests': [{'name': name, 'command': ['test']}
                          for name in required]}
  campaign.validate_inventory(inventory, required)
  bad = copy.deepcopy(inventory)
  bad['tests'].pop()
  rejected(lambda: campaign.validate_inventory(bad, required), 'Missing')
  bad = copy.deepcopy(inventory)
  bad['tests'].append(bad['tests'][0])
  rejected(lambda: campaign.validate_inventory(bad, required), 'Duplicate')
  for index, (code, diagnostic) in enumerate([
      ("print('No tests matched')", 'selection not proven'),
      ("print('running 1 unit tests'); raise SystemExit(1)", 'failure'),
      ("print('WARNING: ThreadSanitizer: data race')", 'Sanitizer'),
      ("print('assertion failed: corruption')", 'failure')]):
    rejected(lambda: campaign.execute(
        [sys.executable, '-c', code], root / f'child-{index}.log', 10,
        expected='running 1 unit tests'), diagnostic)
  campaign.execute([sys.executable, '-c', "print('running 1 unit tests')"],
                    root / 'positive.log', 10, 'running 1 unit tests')
  rejected(lambda: campaign.execute(
      [sys.executable, '-c', "print('1/1 Test #1: probe ... ***Skipped')"],
      root / 'skipped.log', 10,
      expected=r'1/1 Test\s+#\d+: probe\b[^\r\n]*\bPassed\b'),
      'selection not proven')
  try:
    campaign.execute([sys.executable, '-c', 'import time; time.sleep(10)'],
                      root / 'timeout.log', .1)
  except subprocess.TimeoutExpired:
    if 'deadline exceeded' not in (root / 'timeout.log').read_text():
      raise AssertionError('Timeout lost its diagnostic')
  else:
    raise AssertionError('Campaign ignored child timeout')


if __name__ == '__main__':
  with tempfile.TemporaryDirectory(prefix='doba-qualification-') as directory:
    path = Path(directory)
    coverage_controls(path)
    path_controls(path)
    campaign_controls(path)
  print('Coverage and campaign negative controls: PASS')
