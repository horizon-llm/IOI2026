import test from 'node:test';
import assert from 'node:assert/strict';
import { isPlaceholderRun, parseMetric, normalizeRunName, importReference } from '../scripts/import-reference.mjs';
import { validateData } from '../leaderboard/model.js';

// All scores and run identifiers below are synthetic. No reference-page data is embedded.
const capturedAt = '2026-01-01T12:00:00.000Z';
const cell = (text, title = '') => ({ text, title, detail: '' });

function existingData() {
  return {
    schemaVersion: 1, board: 'ioi2026subtasks', title: 'Synthetic board', updatedAt: null,
    source: { label: 'Synthetic evidence', url: 'https://example.org/results' },
    problems: [{ id: 'p1', label: 'P1', name: 'Synthetic problem', day: 1, maxScore: 100, weights: [40, 60] }],
    entries: [{
      id: 'gpt-oss-120b-arbiter', name: 'Synthetic preserved entry', description: 'Synthetic fixture',
      contextTokens: null, k: null, status: 'Published', score: 50, expectedScore: 42.5,
      scoreAt1: null, passAtK: null, passAt1: null, avgThinkTokens: null,
      sourceNote: 'Synthetic fixture; not a measured result.',
      problems: [{
        id: 'p1', score: 50, average: null,
        subtasks: [{ score: 40, average: null }, { score: 10, average: null }],
        submissions: [{ label: 'Synthetic evidence', url: 'https://example.org/submission/1' }],
      }],
    }],
  };
}

function row(runId = 'Model_X_xhigh_128K_pass_8', { context = '128K', k = '8' } = {}) {
  return {
    cells: [
      cell('1'), cell('Model_X_xhigh', runId), cell(context), cell(k), cell('100.0%'),
      cell('30.2 (30.2%)', '30.2345 / 100'), cell('5.1 (5.1%)', '5.1234 / 100'),
      cell('50.00%'), cell('20.00%'), cell('1.2K'),
    ],
    scores: [
      { total: false, title: '*IOI #1 pid=123 max 10.2345/40 avg 2.1234 attempts=8 sub=456', best: '10', average: '2' },
      { total: false, title: '*IOI #2 pid=124 max 20/60 avg 3.0 attempts=8 sub=457', best: '20', average: '3' },
      { total: true, title: 'P1 total max=30.2345/100 avg=5.1234', best: '30', average: '5' },
    ],
  };
}

test('placeholder classification is case-insensitive and restricted to known prefixes', () => {
  for (const id of ['longest_100', 'longest_k50', 'LONGEST_200', 'ojblind_day1_oj_r01', 'OJBLIND_day2_oj_r05', 'smoke_example', 'SMOKE_example']) {
    assert.equal(isPlaceholderRun(id), true, id);
  }
  for (const id of ['Model_X_128K_pass_8', 'Model_longest_128K_pass_8', 'Model_ojblind_128K_pass_8', 'Model_smoke_128K_pass_8']) {
    assert.equal(isPlaceholderRun(id), false, id);
  }
});

test('run names separate base model, reasoning configuration, context and pass count', () => {
  assert.deepEqual(normalizeRunName('Model_X_xhigh_128K_pass_8'), { name: 'Model X', configuration: 'xhigh' });
  assert.deepEqual(normalizeRunName('Model_X_high_131K_pass_50'), { name: 'Model X', configuration: 'high' });
  assert.deepEqual(normalizeRunName('Model_X_low_384K_pass_200'), { name: 'Model X', configuration: 'low' });
  assert.deepEqual(normalizeRunName('Model_X_128K_pass_8'), { name: 'Model X', configuration: '' });
  assert.deepEqual(normalizeRunName('GPT_OSS_120B_t06_131K_pass_50'), { name: 'GPT-OSS-120B', configuration: 't=0.6' });
});

test('malformed run names fail instead of silently inventing a model configuration', () => {
  for (const id of ['', 'Model_X', 'Model_X_128K_pass_', 'Model_X_128K_pass_eight']) assert.throws(() => normalizeRunName(id));
});

test('metric parsing preserves null, zero and exact numeric tooltip precision', () => {
  for (const missing of ['', '—', '  —  ']) assert.equal(parseMetric(cell(missing)), null);
  assert.equal(parseMetric(cell('0')), 0);
  assert.equal(parseMetric(cell('0.00%')), 0);
  assert.equal(parseMetric(cell('42.0%')), 42);
  assert.equal(parseMetric(cell('30.2 (30.2%)', '30.2345 / 100')), 30.2345);
  assert.equal(parseMetric(cell('0', '0 / 100')), 0);
  assert.equal(parseMetric(cell('8', 'Eight synthetic attempts')), 8);
});

test('token parsing expands displayed K values only when requested', () => {
  assert.equal(parseMetric(cell('128K'), { tokens: true }), 128000);
  assert.equal(parseMetric(cell('1.2K'), { tokens: true }), 1200);
  assert.equal(parseMetric(cell('999'), { tokens: true }), 999);
  assert.equal(parseMetric(cell('—'), { tokens: true }), null);
  assert.throws(() => parseMetric(cell('128K')));
});

test('malformed numeric text never becomes a partial numeric result or zero', () => {
  for (const text of ['not measured', '12garbage', 'NaN', 'Infinity', '1.2.3', '']) {
    if (text === '') assert.equal(parseMetric(cell(text)), null);
    else assert.throws(() => parseMetric(cell(text)), text);
  }
});

test('import uses exact tooltip scores and retains the existing public entry', () => {
  const existing = existingData();
  const before = structuredClone(existing);
  const data = importReference({ capturedAt, rows: [row()] }, existing);
  assert.doesNotThrow(() => validateData(data));
  assert.equal(data.entries.length, 2);
  assert.deepEqual(data.entries.find(entry => entry.id === 'gpt-oss-120b-arbiter'), {
    ...before.entries[0], configuration: 'Arbiter', scoreKind: 'observed',
  });
  assert.deepEqual(existing, before, 'Import must not mutate the caller’s existing data');
  const imported = data.entries.find(entry => entry.id !== 'gpt-oss-120b-arbiter');
  assert.equal(imported.name, 'Model X');
  assert.equal(imported.configuration, 'xhigh');
  assert.equal(imported.description, 'Sampling · xhigh');
  assert.equal(imported.scoreKind, 'sampled');
  assert.equal(imported.contextTokens, 128000);
  assert.equal(imported.k, 8);
  assert.equal(imported.score, 30.2345);
  assert.equal(imported.scoreAt1, 5.1234);
  assert.equal(imported.expectedScore, null);
  assert.equal(imported.passAtK, 50);
  assert.equal(imported.passAt1, 20);
  assert.equal(imported.avgThinkTokens, 1200);
  assert.equal(imported.problems[0].score, 30.2345);
  assert.equal(imported.problems[0].average, 5.1234);
  assert.deepEqual(imported.problems[0].subtasks, [{ score: 10.2345, average: 2.1234 }, { score: 20, average: 3 }]);
  assert.equal(data.updatedAt, capturedAt);
});

test('import omits placeholder rows before parsing their intentionally absent measurements', () => {
  const placeholder = id => ({ cells: [cell('99'), cell(id, id)], scores: [] });
  const data = importReference({ capturedAt, rows: [placeholder('longest_100'), placeholder('OJBLIND_example'), placeholder('smoke_example'), row()] }, existingData());
  assert.equal(data.entries.length, 2);
  assert.equal(data.entries.filter(entry => entry.name === 'Model X').length, 1);
});

test('different context and k configurations retain distinct model runs', () => {
  const rows = [
    row('Model_X_xhigh_128K_pass_8'),
    row('Model_X_xhigh_128K_pass_50', { k: '50' }),
    row('Model_X_xhigh_384K_pass_8', { context: '384K' }),
  ];
  const data = importReference({ capturedAt, rows }, existingData());
  const runs = data.entries.filter(entry => entry.name === 'Model X');
  assert.equal(runs.length, 3);
  assert.equal(new Set(runs.map(entry => entry.id)).size, 3);
  assert.deepEqual(runs.map(entry => [entry.contextTokens, entry.k]).sort((a, b) => a[0] - b[0] || a[1] - b[1]), [[128000, 8], [128000, 50], [384000, 8]]);
});

test('import strips source submission identifiers, private links and unrelated existing entries', () => {
  const existing = existingData();
  existing.entries.push({ ...structuredClone(existing.entries[0]), id: 'obsolete-import' });
  const input = row();
  input.cells[1].detail = 'http://10.0.0.1/internal';
  input.scores[0].href = 'http://10.0.0.1/submission/456';
  const data = importReference({ capturedAt, rows: [input] }, existing);
  assert.equal(data.entries.length, 2);
  assert.equal(data.entries.some(entry => entry.id === 'obsolete-import'), false);
  const imported = data.entries.find(entry => entry.id !== 'gpt-oss-120b-arbiter');
  assert.deepEqual(imported.problems[0].submissions, []);
  assert.doesNotMatch(JSON.stringify(data), /10\.0\.0\.1|pid=|sub=|attempts=/);
  assert.match(data.source.url, /^https:\/\/github\.com\/horizon-llm\/IOI2026\//i);
});

test('import rejects invalid timestamps, duplicate runs and missing score cells', () => {
  assert.throws(() => importReference({ capturedAt: 'invalid', rows: [row()] }, existingData()));
  assert.throws(() => importReference({ rows: [row()] }, existingData()));
  assert.throws(() => importReference({ capturedAt, rows: [row(), row()] }, existingData()));
  const missing = row();
  missing.scores.pop();
  assert.throws(() => importReference({ capturedAt, rows: [missing] }, existingData()));
  const extra = row();
  extra.scores.push(structuredClone(extra.scores[0]));
  assert.throws(() => importReference({ capturedAt, rows: [extra] }, existingData()));
});

test('import rejects inconsistent problem totals rather than publishing a misaligned result', () => {
  const input = row();
  input.scores[2].title = 'P1 total max=35/100 avg=5.1234';
  assert.throws(() => importReference({ capturedAt, rows: [input] }, existingData()));
});
