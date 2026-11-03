import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import {
  formatNumber, formatTokens, heatClass, isNumber, metricValue,
  rankEntries, safeUrl, selectEntries, sum, toCsv, validateData,
} from '../leaderboard/model.js';

const published = JSON.parse(await readFile(new URL('../leaderboard/data.json', import.meta.url), 'utf8'));
const clone = value => structuredClone(value);
const arbiter = published.entries.find(entry => entry.id === 'gpt-oss-120b-arbiter');
const expectedSourceRuns = [
  'GPT_6_Astra_xhigh_128K_pass_8',
  'Claude_Fable_5_1_xhigh_384K_pass_8',
  'GPT_6_1_Sol_xhigh_128K_pass_8',
  'Claude_Opus_5_5_xhigh_128K_pass_8',
  'GPT_6_1_Terra_xhigh_128K_pass_8',
  'Claude_Haiku_5_5_xhigh_384K_pass_8',
  'GPT_5_5_xhigh_128K_pass_8',
  'Claude_Fable_5_1_xhigh_128K_pass_8',
  'Qwen3_8_27B_131K_pass_8',
  'Kimi_K3_xhigh_128K_pass_8',
  'GPT_OSS_120B_high_120K_pass_1000',
  'Claude_Haiku_5_5_xhigh_128K_pass_8',
  'GPT_OSS_120B_high_120K_pass_200',
  'Qwen3_6_35B_A3B_t06_131K_pass_1000',
  'GLM_5_3_Flash_low_100K_pass_8',
  'Qwen3_6_35B_A3B_t06_131K_pass_200',
  'Qwen3_6_35B_A3B_t06_131K_pass_100',
  'GPT_OSS_120B_high_120K_pass_50',
  'Qwen3_6_35B_A3B_t06_131K_pass_50',
  'Grok_4_6_128K_pass_8',
  'Qwen3_6_35B_A3B_t06_131K_pass_8',
  'GPT_OSS_120B_high_120K_pass_8',
  'Kimi_K2_5_high_128K_pass_8',
  'Qwen3_6_35B_A3B_131K_pass_8',
];

function fixture() {
  return {
    schemaVersion: 1, board: 'ioi2026subtasks', title: 'Test board', updatedAt: null,
    source: { label: 'Evidence', url: 'https://example.org/results' },
    problems: [{ id: 'p1', label: 'P1', name: 'Problem one', day: 1, maxScore: 100, weights: [40, 60] }],
    entries: [{
      id: 'model-a', name: 'Model A', description: 'Refinement', status: 'Published',
      contextTokens: null, k: null, score: 50, expectedScore: 42.5,
      scoreAt1: null, passAtK: null, passAt1: null, avgThinkTokens: null,
      problems: [{
        id: 'p1', score: 50, average: null,
        subtasks: [{ score: 40, average: null }, { score: 10, average: null }],
        submissions: [{ label: 'program.cpp', url: 'https://example.org/submission/1' }],
      }],
    }],
  };
}

// Parse quoted CSV independently so assertions cover delimiters and embedded lines.
function parseCsv(csv) {
  const rows = [];
  let row = [], value = '', quoted = false;
  for (let i = 0; i < csv.length; i++) {
    const character = csv[i];
    if (character === '"') {
      if (quoted && csv[i + 1] === '"') { value += '"'; i++; }
      else quoted = !quoted;
    } else if (!quoted && character === ',') {
      row.push(value); value = '';
    } else if (!quoted && character === '\r' && csv[i + 1] === '\n') {
      row.push(value); rows.push(row); row = []; value = ''; i++;
    } else value += character;
  }
  assert.equal(quoted, false, 'CSV must close every quoted field');
  assert.equal(value, '', 'CSV must end with a record separator');
  assert.equal(row.length, 0);
  return rows;
}

test('published data validates and retains observed versus expected score precision', () => {
  assert.equal(validateData(published), published);
  assert.equal(published.entries.length, expectedSourceRuns.length + 1);
  const entry = arbiter;
  assert.ok(entry, 'The existing Arbiter result must be preserved');
  assert.equal(entry.score, 390.2028);
  assert.equal(entry.expectedScore, 385.46);
  assert.equal(entry.scoreKind, 'observed');
  assert.equal(entry.configuration, 'Arbiter');
  assert.notEqual(entry.score, entry.expectedScore);
  assert.ok(Math.abs(sum(entry.problems.map(problem => problem.score)) - entry.score) < 1e-9);
  assert.equal(published.problems.length, 6);
  assert.equal(sum(published.problems.map(problem => problem.weights.length)), 41);
  assert.equal(sum(published.problems.map(problem => problem.maxScore)), 600);
  assert.equal(typeof published.updatedAt, 'string');
  assert.ok(Number.isFinite(Date.parse(published.updatedAt)));
  assert.equal(published.source.url, 'https://github.com/horizon-llm/IOI2026/blob/leaderboard/leaderboard/data.json');
  assert.deepEqual(entry.problems.find(problem => problem.id === 'p4').subtasks.map(item => item.score), [4, 6, 9, 13.75, 2.4528]);
  assert.deepEqual(entry.problems.find(problem => problem.id === 'p6').subtasks.map(item => item.score), [5, 4, 7, 10, 7, 12, 17, 0, 0]);
  for (const id of ['p1', 'p2']) {
    assert.ok(entry.problems.find(problem => problem.id === id).subtasks.every(item => item.score === null));
  }
  for (const key of ['contextTokens', 'k', 'scoreAt1', 'passAtK', 'passAt1', 'avgThinkTokens']) assert.equal(entry[key], null);
});

test('published data contains every approved real-model run and no placeholders', () => {
  const imported = published.entries.filter(entry => entry.sourceRun);
  assert.equal(imported.length, expectedSourceRuns.length);
  assert.deepEqual(imported.map(entry => entry.sourceRun).sort(), [...expectedSourceRuns].sort());
  assert.equal(new Set(published.entries.map(entry => entry.id)).size, expectedSourceRuns.length + 1);
  assert.equal(new Set(published.entries.map(entry => entry.name)).size, 14);
  for (const entry of published.entries) {
    assert.doesNotMatch(`${entry.id} ${entry.name} ${entry.sourceRun || ''}`, /(?:longest_|longest-|ojblind_|ojblind-|smoke_|smoke-)/i);
  }
  for (const entry of imported) {
    assert.equal(entry.scoreKind, 'sampled');
    assert.equal(entry.expectedScore, null);
    assert.equal(entry.problems.length, 6);
    assert.equal(sum(entry.problems.map(problem => problem.subtasks.length)), 41);
    assert.ok(entry.problems.every(problem => problem.submissions.length === 0));
  }
  assert.doesNotMatch(JSON.stringify(published), /10\.189\.251\.169|pid=|sub=|judger|sandbox/i);
});

test('published runs retain independent context, sampling and reasoning configurations', () => {
  const haiku = published.entries.filter(entry => entry.name === 'Claude Haiku 5.5');
  assert.deepEqual(haiku.map(entry => entry.contextTokens).sort((a, b) => a - b), [128000, 384000]);
  assert.ok(haiku.every(entry => entry.k === 8 && entry.configuration === 'xhigh'));

  const ossSampling = published.entries.filter(entry => entry.name === 'GPT-OSS-120B' && entry.scoreKind === 'sampled');
  assert.deepEqual(ossSampling.map(entry => entry.k).sort((a, b) => a - b), [8, 50, 200, 1000]);
  assert.ok(ossSampling.every(entry => entry.contextTokens === 120000 && entry.configuration === 'high'));

  const qwen = published.entries.filter(entry => entry.name === 'Qwen3.6-35B-A3B');
  assert.equal(qwen.length, 6);
  assert.deepEqual(qwen.filter(entry => entry.configuration === 't=0.6').map(entry => entry.k).sort((a, b) => a - b), [8, 50, 100, 200, 1000]);
  assert.equal(qwen.filter(entry => entry.configuration === '' && entry.k === 8).length, 1);
  assert.ok(qwen.every(entry => entry.contextTokens === 131000));
});

test('source score precision, pass rates and incomplete judging status survive import', () => {
  const byRun = run => published.entries.find(entry => entry.sourceRun === run);
  const astra = byRun('GPT_6_Astra_xhigh_128K_pass_8');
  assert.equal(astra.score, 600);
  assert.equal(astra.scoreAt1, 578.25);
  assert.equal(astra.passAtK, 100);
  assert.equal(astra.passAt1, 96.65);
  assert.equal(astra.avgThinkTokens, 7800);
  assert.equal(byRun('Claude_Haiku_5_5_xhigh_384K_pass_8').avgThinkTokens, null);
  assert.equal(byRun('GPT_5_5_xhigh_128K_pass_8').status, '99.4% judged');
  assert.equal(byRun('Qwen3_6_35B_A3B_t06_131K_pass_1000').status, '94.6% judged');
  for (const [run, score, scoreAt1, passAtK, passAt1, configuration] of [
    ['GPT_6_1_Sol_xhigh_128K_pass_8', 586, 558.25, 97.56, 93.90, 'xhigh'],
    ['GPT_6_1_Terra_xhigh_128K_pass_8', 520, 496.75, 87.80, 85.67, 'xhigh'],
    ['Kimi_K2_5_high_128K_pass_8', 179, 124.63, 58.54, 43.90, 'high'],
  ]) {
    const entry = byRun(run);
    assert.ok(entry, `Missing newly published run ${run}`);
    assert.equal(entry.score, score);
    assert.equal(entry.scoreAt1, scoreAt1);
    assert.equal(entry.passAtK, passAtK);
    assert.equal(entry.passAt1, passAt1);
    assert.equal(entry.configuration, configuration);
    assert.equal(entry.contextTokens, 128000);
    assert.equal(entry.k, 8);
    assert.equal(entry.avgThinkTokens, null);
    assert.equal(entry.status, '100% judged');
  }
});

test('Fable context variants preserve exact scores, problem totals and the first-place tie', () => {
  const runs = published.entries.filter(entry => entry.name === 'Claude Fable 5.1');
  assert.equal(runs.length, 2);
  const ranks = rankEntries(published.entries);
  for (const [contextTokens, score, scoreAt1, passAtK, passAt1, problemScores] of [
    [384000, 600, 516.13, 100, 90.24, [100, 100, 100, 100, 100, 100]],
    [128000, 446, 421.25, 90.24, 84.45, [40, 100, 100, 44, 100, 62]],
  ]) {
    const entry = runs.find(run => run.contextTokens === contextTokens);
    assert.ok(entry, `Missing Fable context variant ${contextTokens}`);
    assert.equal(entry.sourceRun, `Claude_Fable_5_1_xhigh_${contextTokens / 1000}K_pass_8`);
    assert.equal(entry.configuration, 'xhigh');
    assert.equal(entry.k, 8);
    assert.equal(entry.scoreKind, 'sampled');
    assert.equal(entry.score, score);
    assert.equal(entry.scoreAt1, scoreAt1);
    assert.equal(entry.passAtK, passAtK);
    assert.equal(entry.passAt1, passAt1);
    assert.equal(entry.avgThinkTokens, null);
    assert.equal(entry.status, '100% judged');
    assert.deepEqual(entry.problems.map(problem => problem.score), problemScores);
    if (score === 600) assert.equal(ranks.get(entry.id), 1);
  }
  assert.equal(ranks.get('gpt-6-astra-xhigh-128k-pass-8'), 1);
});

test('unknown values remain distinct from a measured zero', () => {
  assert.equal(isNumber(null), false);
  assert.equal(isNumber(0), true);
  assert.equal(formatNumber(null), '—');
  assert.equal(formatNumber(0), '0');
  assert.equal(formatTokens(null), '—');
  assert.equal(formatTokens(0), '0');
  assert.equal(heatClass(null, 100), 'heat-unknown');
  assert.equal(heatClass(0, 100), 'heat-zero');
  assert.equal(heatClass(25, 100), 'heat-partial');
  assert.equal(heatClass(100, 100), 'heat-full');
  const data = fixture();
  Object.assign(data.entries[0], { score: 0, expectedScore: null, scoreAt1: 0, passAtK: 0, passAt1: 0 });
  Object.assign(data.entries[0].problems[0], { score: 0, average: 0, subtasks: [{ score: 0, average: 0 }, { score: null, average: null }] });
  assert.doesNotThrow(() => validateData(data));
});

test('validation rejects malformed structure, inconsistent totals, and unsafe evidence links', async t => {
  const cases = [
    ['unsupported schema', data => { data.schemaVersion = 2; }],
    ['missing entries', data => { delete data.entries; }],
    ['empty problems', data => { data.problems = []; }],
    ['invalid update timestamp', data => { data.updatedAt = 'not-a-date'; }],
    ['unsafe source URL', data => { data.source.url = 'javascript:alert(1)'; }],
    ['unsafe submission URL', data => { data.entries[0].problems[0].submissions[0].url = 'data:text/html,<script>bad</script>'; }],
    ['duplicate entry ID', data => { data.entries.push(clone(data.entries[0])); }],
    ['duplicate problem ID', data => { data.problems.push(clone(data.problems[0])); }],
    ['mismatched result ID', data => { data.entries[0].problems[0].id = 'unknown'; }],
    ['missing result', data => { data.entries[0].problems = []; }],
    ['wrong subtask count', data => { data.entries[0].problems[0].subtasks.pop(); }],
    ['invalid weight total', data => { data.problems[0].weights = [40, 59]; }],
    ['negative weight', data => { data.problems[0].weights = [-1, 101]; }],
    ['inconsistent subtask total', data => { data.entries[0].problems[0].subtasks[1].score = 9; }],
    ['inconsistent overall total', data => { data.entries[0].score = 49; }],
    ['subtask average exceeds best', data => { data.entries[0].problems[0].subtasks[1].average = 11; }],
    ['fractional k', data => { data.entries[0].k = 1.5; }],
    ['zero k', data => { data.entries[0].k = 0; }],
  ];
  for (const [name, mutate] of cases) await t.test(name, () => {
    const data = fixture();
    mutate(data);
    assert.throws(() => validateData(data));
  });
});

test('validation rejects nonnumeric, negative, and out-of-range measurements', async t => {
  for (const value of [NaN, Infinity, -1, 101, '50', undefined]) {
    await t.test(`invalid score ${String(value)}`, () => {
      const data = fixture();
      data.entries[0].score = value;
      assert.throws(() => validateData(data));
    });
  }
  const data = fixture();
  data.entries[0].passAtK = 100.1;
  assert.throws(() => validateData(data));
});

test('validation rejects a problem average greater than its best score', () => {
  const data = fixture();
  data.entries[0].problems[0].average = 51;
  assert.throws(() => validateData(data), /average|best|score/i);
});

test('validation rejects a single-attempt average greater than the observed best total', () => {
  const data = fixture();
  data.entries[0].scoreAt1 = 51;
  assert.throws(() => validateData(data), /average|best|score/i);
});

test('validation accepts an empty published board', () => {
  const data = fixture();
  data.entries = [];
  assert.doesNotThrow(() => validateData(data));
});

test('HTTPS evidence URLs are allowed while active and local schemes are rejected', () => {
  assert.equal(safeUrl('https://qoj.ac/submission/2809712'), 'https://qoj.ac/submission/2809712');
  for (const url of ['javascript:alert(1)', 'data:text/html,hi', 'file:///tmp/a', 'http://example.org', '/relative', null]) assert.equal(safeUrl(url), null);
});

test('metric lookup preserves zero and reports absent values as null', () => {
  const entry = { score: 0, problems: [{ id: 'p1', score: 0 }, { id: 'p2', score: null }] };
  assert.equal(metricValue(entry, 'score'), 0);
  assert.equal(metricValue(entry, 'problem:p1'), 0);
  assert.equal(metricValue(entry, 'problem:p2'), null);
  assert.equal(metricValue(entry, 'problem:p3'), null);
  assert.equal(metricValue(entry, 'scoreAt1'), null);
});

const sortableEntries = () => [
  { id: 'unknown', name: 'Unknown', description: 'Unmeasured', score: null, problems: [{ id: 'p1', score: null }] },
  { id: 'beta', name: 'Beta', description: 'Refinement', score: 10, problems: [{ id: 'p1', score: 7 }] },
  { id: 'zero', name: 'Zero', description: 'Baseline', score: 0, problems: [{ id: 'p1', score: 0 }] },
  { id: 'alpha', name: 'Alpha', description: 'Sampling', score: 10, problems: [{ id: 'p1', score: 3 }] },
];

test('sorting keeps missing measurements last in both directions and retains zero', () => {
  const entries = sortableEntries();
  const original = clone(entries);
  assert.deepEqual(selectEntries(entries, '', 'score', 'desc').map(entry => entry.id), ['alpha', 'beta', 'zero', 'unknown']);
  assert.deepEqual(selectEntries(entries, '', 'score', 'asc').map(entry => entry.id), ['zero', 'alpha', 'beta', 'unknown']);
  assert.deepEqual(selectEntries(entries, '', 'problem:p1', 'desc').map(entry => entry.id), ['beta', 'alpha', 'zero', 'unknown']);
  assert.deepEqual(selectEntries(entries, '', 'problem:p1', 'asc').map(entry => entry.id), ['zero', 'alpha', 'beta', 'unknown']);
  assert.deepEqual(entries, original, 'Selection must not reorder or edit the input data');
});

test('search matches model names and descriptions, ignoring case and edge whitespace', () => {
  const entries = sortableEntries();
  assert.deepEqual(selectEntries(entries, '  ALpHa  ').map(entry => entry.id), ['alpha']);
  assert.deepEqual(selectEntries(entries, 'REFINEMENT').map(entry => entry.id), ['beta']);
  assert.deepEqual(selectEntries(entries, 'missing'), []);
  assert.equal(selectEntries(entries, ' ').length, entries.length);
});

test('rank uses competition ties, preserves zero rank, and leaves missing score unranked', () => {
  const ranks = rankEntries(sortableEntries());
  assert.equal(ranks.get('alpha'), 1);
  assert.equal(ranks.get('beta'), 1);
  assert.equal(ranks.get('zero'), 3);
  assert.equal(ranks.get('unknown'), '—');
  assert.deepEqual([...rankEntries([])], []);
});

test('CSV exports raw score precision, null as empty, and zero as zero', () => {
  const [header, row] = parseCsv(toCsv(published, [arbiter]));
  const at = name => row[header.indexOf(name)];
  assert.equal(at('Score'), '390.2028');
  assert.equal(at('Expected contest score'), '385.46');
  assert.equal(at('P4 S5 best'), '2.4528');
  assert.equal(at('P6 S8 best'), '0');
  assert.equal(at('P1 S1 best'), '');
  assert.equal(at('Score@1'), '');
  assert.equal(at('Pass@k (%)'), '');
  assert.equal(row.length, header.length);
  assert.equal(parseCsv(toCsv(published, [])).length, 1);
});

test('CSV preserves commas, quotes and embedded newlines without adding columns', () => {
  const data = fixture();
  data.entries[0].name = 'Model, "quoted"\r\nsecond line';
  data.entries[0].description = 'Line one\nLine two';
  const [header, row] = parseCsv(toCsv(data, data.entries));
  assert.equal(row.length, header.length);
  assert.equal(row[0], data.entries[0].name);
  assert.equal(row[1], data.entries[0].description);
});

test('CSV neutralizes formula-like text, including leading whitespace', () => {
  for (const value of ['=1+1', '+SUM(A1)', '-1+2', '@SUM(A1)', ' \t=HYPERLINK("https://example.org")', '\r\n=1+1']) {
    const data = fixture();
    data.entries[0].name = value;
    data.entries[0].description = value;
    data.entries[0].status = value;
    data.problems[0].label = value;
    const [header, row] = parseCsv(toCsv(data, data.entries));
    assert.equal(row[0], `'${value}`);
    assert.equal(row[1], `'${value}`);
    assert.equal(row[4], `'${value}`);
    assert.equal(header[11], `'${value} score`);
    assert.equal(row[5], '50', 'Numeric measurements must remain numeric text');
  }
});
