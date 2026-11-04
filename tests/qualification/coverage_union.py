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

from html import escape
from pathlib import Path


# +---------------------------------------------------------------------------+
# | [>] production_name                                          ( function ) |
# +---------------------------------------------------------------------------+
def production_name(filename, root):
  try:
    name = Path(filename).resolve().relative_to(root).as_posix()
  except ValueError:
    return None
  return name if name.startswith('include/') else None


# +---------------------------------------------------------------------------+
# | [>] merge                                                    ( function ) |
# +---------------------------------------------------------------------------+
def merge(files, document, lcov, root):
  # Canonicalize the root once, including Windows 8.3 temporary aliases.
  root = root.resolve()
  # Coverage is a union of source locations, not an average of executables or
  # instantiations. Every object was exported with only its own raw profiles.
  for data in document['data']:
    for item in data['files']:
      name = production_name(item['filename'], root)
      if name is None:
        continue
      file = files.setdefault(name, {'lines': {}, 'branches': {}})
      for branch in item.get('branches', []):
        key = tuple(branch[:4]) + (branch[8],)
        values = file['branches'].setdefault(key, [False, False])
        values[0] |= branch[4] > 0
        values[1] |= branch[5] > 0
  name = None
  for line in lcov.splitlines():
    if line.startswith('SF:'):
      name = production_name(line[3:], root)
    elif line == 'end_of_record':
      name = None
    elif name is not None and line.startswith('DA:'):
      number, count = map(int, line[3:].split(',')[:2])
      lines = files.setdefault(name, {'lines': {}, 'branches': {}})['lines']
      lines[number] = max(lines.get(number, 0), count)


# +---------------------------------------------------------------------------+
# | [>] summarize                                                ( function ) |
# +---------------------------------------------------------------------------+
def summarize(files, root, module):
  if not files:
    raise RuntimeError('No production coverage found')
  groups, uncovered = {}, []
  for name, file in sorted(files.items()):
    group = groups.setdefault(module(name), {
        'files': [], 'lines': {'count': 0, 'covered': 0},
        'branches': {'count': 0, 'covered': 0}})
    group['files'].append(name)
    group['lines']['count'] += len(file['lines'])
    group['lines']['covered'] += sum(n > 0 for n in file['lines'].values())
    group['branches']['count'] += 2 * len(file['branches'])
    for position, outcomes in file['branches'].items():
      group['branches']['covered'] += sum(outcomes)
      if not all(outcomes):
        uncovered.append({'file': name, 'line': position[0],
                          'column': position[1], 'true': outcomes[0],
                          'false': outcomes[1]})
  for group in groups.values():
    for metric in ('lines', 'branches'):
      counts = group[metric]
      counts['percent'] = (100 * counts['covered'] / counts['count']
                           if counts['count'] else None)
  headers = {p.relative_to(root).as_posix()
             for p in (root / 'include').rglob('*.h')}
  return {'metric': 'union of executable source lines and condition outcomes',
          'modules': groups, 'measured_headers': sorted(files),
          'unmapped_headers': sorted(headers - files.keys()),
          'uncovered_branch_outcomes': uncovered}


# +---------------------------------------------------------------------------+
# | [>] html_report                                              ( function ) |
# +---------------------------------------------------------------------------+
def html_report(files, summary, root, output):
  output.mkdir()
  style = ('<style>body{font:16px system-ui;margin:2rem}'
           'pre{font:13px monospace;line-height:1.5}'
           '.miss{background:#ffd6d6}.hit{background:#d5f5df}'
           'td,th{padding:.4rem;text-align:left}</style>')
  index = ['<!doctype html><meta charset="utf-8">', style,
           '<title>Doba Release coverage</title><h1>Release coverage</h1>',
           '<p>Union by source line and condition outcome. Unmapped headers '
           'and per-executable LLVM exports are listed in summary.json.</p>',
           '<table><tr><th>Module</th><th>Lines</th><th>Outcomes</th></tr>']
  for name, group in summary['modules'].items():
    values = []
    for key in ('lines', 'branches'):
      metric = group[key]
      values.append(f"{metric['covered']}/{metric['count']}")
    index.append('<tr><td>' + escape(name) + '</td><td>' +
                 '</td><td>'.join(values) + '</td></tr>')
  index.append('</table><h2>Source files</h2><ul>')
  for number, (name, file) in enumerate(sorted(files.items())):
    target = f'file-{number}.html'
    index.append(f'<li><a href="{target}">{escape(name)}</a></li>')
    content = ['<!doctype html><meta charset="utf-8">', style,
               '<title>' + escape(name) + '</title>',
               '<h1>' + escape(name) + '</h1><pre>']
    for line, text in enumerate(
        (root / name).read_text(encoding='utf-8').splitlines(), 1):
      count = file['lines'].get(line)
      css = '' if count is None else ('hit' if count else 'miss')
      content.append(f'<span id="L{line}" class="{css}">' +
                     f'{line:5} {escape(text)}</span>\n')
    content.append('</pre><h2>Condition outcomes</h2><ul>')
    for position, outcomes in sorted(file['branches'].items()):
      content.append(f'<li><a href="#L{position[0]}">'
                     f'{position[0]}:{position[1]}</a> '
                     f'true={outcomes[0]}, false={outcomes[1]}</li>')
    content.append('</ul>')
    (output / target).write_text(''.join(content), encoding='utf-8')
  index.append('</ul>')
  (output / 'index.html').write_text(''.join(index), encoding='utf-8')


# +---------------------------------------------------------------------------+
# | [>] markdown_report                                          ( function ) |
# +---------------------------------------------------------------------------+
def markdown_report(summary):
  rows = ['# Release source coverage', '',
          '| Module | Lines | Condition outcomes |', '| --- | --- | --- |']
  for name, group in summary['modules'].items():
    values = []
    for key in ('lines', 'branches'):
      metric = group[key]
      percent = metric['percent']
      value = f'{percent:.2f}%' if percent is not None else 'not applicable'
      values.append(f"{metric['covered']}/{metric['count']} ({value})")
    rows.append('| ' + name + ' | ' + ' | '.join(values) + ' |')
  rows.extend(['', f"Unmapped headers: {len(summary['unmapped_headers'])}.",
               'See summary.json for the inventory and uncovered outcomes.',
               'Download the coverage artifact for html/index.html.',
               'Coverage is not a security certification.', ''])
  return '\n'.join(rows)
