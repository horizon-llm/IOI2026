export const isNumber = value => typeof value === 'number' && Number.isFinite(value);
export const sum = values => values.reduce((total, value) => total + value, 0);
export const formatNumber = (value, digits = 2) => isNumber(value)
  ? value.toLocaleString('en-US', { maximumFractionDigits: digits }) : '—';
export const formatTokens = value => !isNumber(value) ? '—'
  : value >= 1000 ? `${formatNumber(value / 1000, 1)}K` : formatNumber(value, 0);
export const heatClass = (score, max) => !isNumber(score) ? 'heat-unknown'
  : score === 0 ? 'heat-zero' : score >= max - 1e-7 ? 'heat-full' : 'heat-partial';

export function safeUrl(value) {
  try { const url = new URL(value); return url.protocol === 'https:' ? url.href : null; }
  catch { return null; }
}

/** Fail closed on inconsistent scores; unknown values must remain null. */
export function validateData(data) {
  const check = (condition, message) => { if (!condition) throw new Error(message); };
  const bounded = (value, max, label) => check(value === null || (isNumber(value) && value >= 0 && value <= max + 1e-6), `Invalid ${label}`);
  const close = (a, b) => Math.abs(a - b) < 0.015;
  check(data?.schemaVersion === 1, 'Unsupported data format');
  check(Array.isArray(data.problems) && data.problems.length > 0, 'Missing problems');
  check(Array.isArray(data.entries), 'Missing entries');
  check(typeof data.title === 'string' && safeUrl(data.source?.url), 'Missing source');
  check(data.updatedAt === null || (typeof data.updatedAt === 'string' && Number.isFinite(Date.parse(data.updatedAt))), 'Invalid update date');
  const problemIds = new Set();
  for (const problem of data.problems) {
    check(typeof problem.id === 'string' && !problemIds.has(problem.id), 'Duplicate or missing problem ID');
    problemIds.add(problem.id);
    check(typeof problem.label === 'string' && typeof problem.name === 'string', 'Missing problem name');
    check(isNumber(problem.maxScore) && problem.maxScore > 0, 'Invalid problem maximum');
    check(Array.isArray(problem.weights) && problem.weights.every(weight => isNumber(weight) && weight > 0), 'Invalid subtask weights');
    check(close(sum(problem.weights), problem.maxScore), 'Subtask weights do not match problem maximum');
  }
  const max = sum(data.problems.map(problem => problem.maxScore));
  const ids = new Set();
  for (const entry of data.entries) {
    check(typeof entry.id === 'string' && !ids.has(entry.id), 'Duplicate or missing entry ID');
    ids.add(entry.id);
    check(typeof entry.name === 'string' && entry.name.length > 0, 'Missing model name');
    for (const key of ['score', 'expectedScore', 'scoreAt1']) bounded(entry[key], max, key);
    check(entry.scoreAt1 === null || entry.score === null || entry.scoreAt1 <= entry.score + 1e-6, 'Average score exceeds best score');
    for (const key of ['passAtK', 'passAt1']) bounded(entry[key], 100, key);
    for (const key of ['contextTokens', 'k', 'avgThinkTokens']) bounded(entry[key], Number.MAX_SAFE_INTEGER, key);
    check(entry.k === null || (Number.isInteger(entry.k) && entry.k > 0), 'Invalid k');
    check(Array.isArray(entry.problems) && entry.problems.length === data.problems.length, 'Missing problem results');
    check(new Set(entry.problems.map(problem => problem.id)).size === data.problems.length, 'Duplicate problem results');
    for (const problem of data.problems) {
      const result = entry.problems.find(item => item.id === problem.id);
      check(result, 'Problem result ID mismatch');
      bounded(result.score, problem.maxScore, 'problem score');
      bounded(result.average, problem.maxScore, 'problem average');
      check(result.average === null || result.score === null || result.average <= result.score + 1e-6, 'Problem average exceeds best score');
      check(Array.isArray(result.subtasks) && result.subtasks.length === problem.weights.length, 'Subtask count mismatch');
      result.subtasks.forEach((subtask, index) => {
        bounded(subtask.score, problem.weights[index], 'subtask score');
        bounded(subtask.average, problem.weights[index], 'subtask average');
        check(subtask.average === null || subtask.score === null || subtask.average <= subtask.score + 1e-6, 'Subtask average exceeds best score');
      });
      if (isNumber(result.score) && result.subtasks.every(item => isNumber(item.score))) {
        check(close(sum(result.subtasks.map(item => item.score)), result.score), 'Subtask scores do not match problem score');
      }
      check(Array.isArray(result.submissions) && result.submissions.every(item => typeof item.label === 'string' && safeUrl(item.url)), 'Invalid submission link');
    }
    if (isNumber(entry.score) && entry.problems.every(problem => isNumber(problem.score))) {
      check(close(sum(entry.problems.map(problem => problem.score)), entry.score), 'Problem scores do not match total');
    }
  }
  return data;
}

export function metricValue(entry, key) {
  if (key.startsWith('problem:')) return entry.problems.find(problem => problem.id === key.slice(8))?.score ?? null;
  return entry[key] ?? null;
}

export function selectEntries(entries, query = '', key = 'score', direction = 'desc') {
  const needle = query.trim().toLocaleLowerCase();
  return entries.filter(entry => `${entry.name} ${entry.description || ''}`.toLocaleLowerCase().includes(needle))
    .sort((a, b) => {
      const left = metricValue(a, key), right = metricValue(b, key);
      if (left === null || right === null) return left === right ? a.name.localeCompare(b.name) : left === null ? 1 : -1;
      const result = typeof left === 'string' ? left.localeCompare(right) : left - right;
      return (direction === 'asc' ? result : -result) || a.name.localeCompare(b.name);
    });
}

export function rankEntries(entries) {
  const sorted = selectEntries(entries);
  return new Map(sorted.map((entry, index) => [entry.id, isNumber(entry.score)
    ? sorted.findIndex(other => other.score === entry.score) + 1 : '—']));
}

export function toCsv(data, entries) {
  const header = ['Model', 'Description', 'Context tokens', 'k', 'Status', 'Observed score', 'Expected contest score', 'Score@1', 'Pass@k (%)', 'Pass@1 (%)', 'Average thinking tokens'];
  for (const problem of data.problems) {
    header.push(`${problem.label} score`, `${problem.label} average`);
    problem.weights.forEach((_, index) => header.push(`${problem.label} S${index + 1} best`, `${problem.label} S${index + 1} average`));
  }
  const rows = entries.map(entry => {
    const row = [entry.name, entry.description, entry.contextTokens, entry.k, entry.status, entry.score, entry.expectedScore, entry.scoreAt1, entry.passAtK, entry.passAt1, entry.avgThinkTokens];
    for (const problem of data.problems) {
      const result = entry.problems.find(item => item.id === problem.id);
      row.push(result.score, result.average);
      result.subtasks.forEach(item => row.push(item.score, item.average));
    }
    return row;
  });
  const escape = value => {
    let text = value == null ? '' : String(value);
    if (typeof value === 'string' && /^[\s]*[=+@-]/.test(text)) text = `'${text}`;
    return `"${text.replaceAll('"', '""')}"`;
  };
  return [header, ...rows].map(row => row.map(escape).join(',')).join('\r\n') + '\r\n';
}
