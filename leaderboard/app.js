import { formatNumber, formatTokens, heatClass, isNumber, rankEntries, safeUrl, selectEntries, sum, toCsv, validateData } from './model.js';

const $ = selector => document.querySelector(selector);
const escape = value => String(value ?? '').replace(/[&<>"']/g, character => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[character]);
let data = null;
let paused = false;
let loading = false;
let lastChecked = null;
let lastError = false;
let sortKey = 'score';
let direction = 'desc';
const columns = [
  { key: 'name', label: 'Model name', className: 'model-cell' },
  { key: 'contextTokens', label: 'Max tokens' },
  { key: 'k', label: 'k' },
  { key: 'score', label: 'Score', title: 'Observed total: sum of best scores on each subtask. See result details for the evaluation setup.' },
  { key: 'scoreAt1', label: 'Score@1', title: 'Average score across single attempts, when reported. This is not the expected contest score.' },
  { key: 'passAtK', label: 'Pass@k' },
  { key: 'passAt1', label: 'Pass@1' },
  { key: 'avgThinkTokens', label: 'Avg think' },
];

function scoreHtml(score, average) {
  return `<span class="score-value">${formatNumber(score)}</span>${isNumber(average) ? `<span class="score-secondary">${formatNumber(average, 1)}</span>` : ''}`;
}

function metricHtml(entry, key, max) {
  const value = entry[key];
  if (key === 'name') return `<button type="button" class="model-name" data-entry="${escape(entry.id)}">${escape(entry.name)}</button>${entry.configuration ? `<span class="model-caption">${escape(entry.configuration)}</span>` : ''}`;
  if (key === 'score') return `<span class="score-value" title="${entry.scoreKind === 'sampled' ? 'Score@k' : 'Observed contest score'}">${formatNumber(value)}</span><span class="score-secondary">${isNumber(value) ? `${formatNumber(value / max * 100, 1)}%` : '—'}</span>`;
  if (key === 'contextTokens' || key === 'avgThinkTokens') return formatTokens(value);
  if (key === 'passAtK' || key === 'passAt1') return isNumber(value) ? `${formatNumber(value)}%` : '—';
  return formatNumber(value);
}

function sortHeader(key, label, className = 'metric-cell', attributes = '', title = '') {
  const selected = sortKey === key;
  return `<th scope="col" class="${className}" ${attributes} aria-sort="${selected ? direction === 'asc' ? 'ascending' : 'descending' : 'none'}"><button type="button" data-sort="${escape(key)}" title="${escape(title || `Sort by ${label}`)}">${escape(label)} <span aria-hidden="true">${selected ? direction === 'asc' ? '↑' : '↓' : '↕'}</span></button></th>`;
}

function render() {
  const focused = document.activeElement;
  const focusSort = focused?.dataset.sort;
  const focusEntry = focused?.dataset.entry;
  if (!data) return;
  const rows = selectEntries(data.entries, $('#search').value, sortKey, direction);
  const ranks = rankEntries(data.entries);
  const max = sum(data.problems.map(problem => problem.maxScore));
  const visibleColumns = columns.filter(column => ['name', 'score'].includes(column.key) || data.entries.some(entry => isNumber(entry[column.key])));
  let head = '<tr><th scope="col" class="rank-cell" rowspan="2">#</th>';
  head += visibleColumns.map(column => sortHeader(column.key, column.label, column.className, 'rowspan="2"', column.title)).join('');
  head += data.problems.map(problem => `<th scope="colgroup" colspan="${problem.weights.length + 1}" title="${escape(problem.name)}">${escape(problem.label)} <span class="score-secondary">${escape(problem.name)} · ${problem.maxScore} pts</span></th>`).join('');
  head += '</tr>';
  head += '<tr>' + data.problems.map(problem => problem.weights.map((weight, index) => `<th scope="col" class="subtask-cell" title="${escape(problem.label)} S${index + 1}: ${weight} points">S${index + 1}<span class="score-secondary">${weight}</span></th>`).join('') + sortHeader(`problem:${problem.id}`, 'Total', 'group-total')).join('') + '</tr>';
  $('#leaderboard thead').innerHTML = head;
  $('#leaderboard tbody').innerHTML = rows.map(entry => {
    let row = `<tr><td class="rank-cell">${ranks.get(entry.id)}</td>`;
    row += visibleColumns.map(column => `<td class="${column.className || 'metric-cell'}">${metricHtml(entry, column.key, max)}</td>`).join('');
    row += data.problems.map(problem => {
      const result = entry.problems.find(item => item.id === problem.id);
      const total = `<td class="group-total ${heatClass(result.score, problem.maxScore)}" title="${escape(problem.name)}: best ${formatNumber(result.score, 4)}/${problem.maxScore}; average ${formatNumber(result.average, 4)}">${scoreHtml(result.score, result.average)}</td>`;
      return result.subtasks.map((subtask, index) => `<td class="subtask-cell ${heatClass(subtask.score, problem.weights[index])}" title="${escape(problem.label)} S${index + 1}: best ${formatNumber(subtask.score, 4)}/${problem.weights[index]}; average ${formatNumber(subtask.average, 4)}">${scoreHtml(subtask.score, subtask.average, problem.weights[index])}</td>`).join('') + total;
    }).join('');
    return row + '</tr>';
  }).join('');
  $('#empty-state').hidden = rows.length > 0;
  $('#empty-state').textContent = data.entries.length ? 'No models match your search. Try a different name.' : 'No results have been published yet.';
  $('#result-count').textContent = `Showing ${rows.length} of ${data.entries.length} model ${data.entries.length === 1 ? 'run' : 'runs'}`;
  $('#board-meta').textContent = `${data.problems.length} problems · ${sum(data.problems.map(problem => problem.weights.length))} subtask groups · ${max} points`;
  $('#source-link').href = safeUrl(data.source.url);
  $('#export').disabled = rows.length === 0;
  if (focusSort || focusEntry) {
    [...document.querySelectorAll('[data-sort], [data-entry]')].find(button =>
      focusSort ? button.dataset.sort === focusSort : button.dataset.entry === focusEntry)?.focus({ preventScroll: true });
  }
}

function updateStatus() {
  $('.status-dot').dataset.status = lastError ? 'error' : paused ? 'paused' : 'ready';
  const time = lastChecked?.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  const publication = data?.updatedAt ? ` · Data updated ${new Date(data.updatedAt).toLocaleDateString()}` : ' · Published snapshot';
  $('#refresh-status').textContent = lastError
    ? data ? `Refresh failed · Showing last loaded results (${time})` : 'Unable to load results. Select Refresh now to retry.'
    : `${paused ? 'Paused' : 'Checked'}${time ? ` ${time}` : ''}${data ? publication : ''}`;
  $('#pause').textContent = paused ? 'Resume' : 'Pause';
  $('#pause').setAttribute('aria-pressed', String(paused));
}

async function refresh() {
  if (loading) return;
  loading = true;
  $('#refresh').disabled = true;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 10000);
  try {
    const response = await fetch(new URL('./data.json', import.meta.url), { cache: 'no-store', signal: controller.signal });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const next = validateData(await response.json());
    const changed = JSON.stringify(next) !== JSON.stringify(data);
    data = next;
    lastChecked = new Date();
    lastError = false;
    if (changed) render();
  } catch (error) {
    console.error('Could not load leaderboard data:', error);
    lastError = true;
    if (!data) {
      $('#empty-state').textContent = 'Results could not be loaded. Check your connection and try again.';
      $('#empty-state').hidden = false;
      $('#result-count').textContent = 'Results unavailable';
    }
  } finally {
    clearTimeout(timeout);
    loading = false;
    $('#refresh').disabled = false;
    updateStatus();
  }
}

function openDetails(id) {
  const entry = data.entries.find(item => item.id === id);
  if (!entry) return;
  $('#detail-title').textContent = entry.description ? `${entry.name} · ${entry.description}` : entry.name;
  const maxScore = sum(data.problems.map(problem => problem.maxScore));
  const summary = [{ label: entry.scoreKind === 'sampled' ? 'Score@k' : 'Observed score', value: entry.score },
    { label: 'Score@1', value: entry.scoreAt1 }, { label: 'Expected contest score', value: entry.expectedScore }]
    .filter(metric => isNumber(metric.value)).map(metric => `<div><span>${metric.label}</span><strong>${formatNumber(metric.value, 4)} / ${maxScore}</strong></div>`).join('');
  const run = entry.sourceRun ? `<p class="detail-note">Run: <code>${escape(entry.sourceRun)}</code></p>` : '';
  $('#detail-body').innerHTML = `<div class="detail-summary">${summary}</div>${run}<p class="detail-note">${escape(entry.sourceNote || entry.description || '')}</p><div class="detail-grid">${data.problems.map(problem => {
    const result = entry.problems.find(item => item.id === problem.id);
    return `<section class="detail-problem"><h3>${escape(problem.label)} · ${escape(problem.name)}</h3><p><strong>${formatNumber(result.score, 4)}</strong> / ${problem.maxScore} points</p><div class="detail-subtasks">${result.subtasks.map((subtask, index) => `<span class="${heatClass(subtask.score, problem.weights[index])}" title="${escape(problem.label)} S${index + 1}"><small>S${index + 1}</small><strong>${formatNumber(subtask.score, 4)}</strong><small>/ ${problem.weights[index]}</small></span>`).join('')}</div><ul class="submission-list">${result.submissions.map(submission => `<li><a href="${escape(safeUrl(submission.url))}" target="_blank" rel="noopener noreferrer">${escape(submission.label)} · QOJ ↗</a></li>`).join('')}</ul></section>`;
  }).join('')}</div>`;
  $('#detail-dialog').showModal();
}

$('#leaderboard').addEventListener('click', event => {
  const sort = event.target.closest('[data-sort]');
  if (sort) {
    const key = sort.dataset.sort;
    direction = key === sortKey ? direction === 'desc' ? 'asc' : 'desc' : key === 'name' ? 'asc' : 'desc';
    sortKey = key;
    render();
    [...document.querySelectorAll('[data-sort]')].find(button => button.dataset.sort === key)?.focus({ preventScroll: true });
  }
  const model = event.target.closest('[data-entry]');
  if (model) openDetails(model.dataset.entry);
});
$('#search').addEventListener('input', render);
$('#close-dialog').addEventListener('click', () => $('#detail-dialog').close());
$('#pause').addEventListener('click', () => { paused = !paused; updateStatus(); if (!paused) refresh(); });
$('#refresh').addEventListener('click', refresh);
$('#export').addEventListener('click', () => {
  const csv = toCsv(data, selectEntries(data.entries, $('#search').value, sortKey, direction));
  const url = URL.createObjectURL(new Blob(['\uFEFF', csv], { type: 'text/csv;charset=utf-8' }));
  const link = document.createElement('a');
  link.href = url;
  link.download = 'ioi2026-leaderboard.csv';
  link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
});
setInterval(() => { if (!paused && !document.hidden) refresh(); }, 15000);
render();
refresh();
