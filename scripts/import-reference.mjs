import { readFileSync, writeFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';
import { validateData } from '../leaderboard/model.js';

export const isPlaceholderRun = runId => /^(?:longest_|ojblind_|smoke_)/i.test(runId);

export function parseMetric(cell, { tokens = false } = {}) {
  const text = String(cell?.text ?? '').trim();
  if (!text || text === '—' || text === '-') return null;
  const precise = String(cell?.title ?? '').match(/^(\d+(?:\.\d+)?)\s*\//);
  if (precise) return Number(precise[1]);
  const match = text.match(/^(\d+(?:\.\d+)?)(K)?(?:%|\s*\([^)]*\))?$/i);
  if (!match || (match[2] && !tokens)) throw new Error(`Invalid metric: ${text}`);
  return Number(match[1]) * (match[2] ? 1000 : 1);
}

export function normalizeRunName(runId) {
  if (typeof runId !== 'string' || !/^.+_\d+(?:\.\d+)?K_pass_[1-9]\d*$/i.test(runId)) throw new Error('Invalid model run ID');
  let base = runId.replace(/_\d+(?:\.\d+)?K_pass_\d+$/i, '');
  const match = base.match(/_(xhigh|high|medium|low|t\d+)$/i);
  let configuration = '';
  if (match) {
    base = base.slice(0, -match[0].length);
    configuration = match[1].toLowerCase();
    if (/^t\d+$/.test(configuration)) configuration = `t=${Number(configuration.slice(1)) / 10}`;
  }
  const names = {
    GPT_6_Astra: 'GPT-6 Astra', GPT_6_1_Sol: 'GPT-6.1 Sol', GPT_6_1_Terra: 'GPT-6.1 Terra',
    GPT_5_5: 'GPT-5.5', GPT_OSS_120B: 'GPT-OSS-120B',
    Claude_Opus_5_5: 'Claude Opus 5.5', Claude_Haiku_5_5: 'Claude Haiku 5.5', Claude_Fable_5_1: 'Claude Fable 5.1',
    Qwen3_8_27B: 'Qwen3.8-27B', Qwen3_6_35B_A3B: 'Qwen3.6-35B-A3B',
    GLM_5_3_Flash: 'GLM-5.3 Flash', Kimi_K3: 'Kimi K3', Kimi_K2_5: 'Kimi K2.5', Grok_4_6: 'Grok 4.6',
  };
  return { name: names[base] ?? base.replaceAll('_', ' '), configuration };
}

function parseScoreBox(box, weight, total) {
  if (!box || box.total !== total) throw new Error('Unexpected subtask/total order');
  const title = String(box.title ?? '');
  const score = title.match(/\bmax[=\s]+(\d+(?:\.\d+)?)\/(\d+(?:\.\d+)?)/);
  const average = title.match(/\bavg[=\s]+(\d+(?:\.\d+)?)/);
  if (!score || !average || Number(score[2]) !== weight) throw new Error('Invalid score box or subtask weight');
  return { score: Number(score[1]), average: Number(average[1]) };
}

/** Convert a reviewed DOM snapshot. Operational metadata and internal links are omitted. */
export function importReference(snapshot, existingData) {
  if (!Array.isArray(snapshot.rows) || !Number.isFinite(Date.parse(snapshot.capturedAt))) throw new Error('Invalid reference snapshot');
  const next = structuredClone(existingData);
  const imported = snapshot.rows.filter(row => {
    const runId = row.cells?.[1]?.title || row.cells?.[1]?.text;
    if (typeof runId !== 'string' || !runId) throw new Error('Missing run ID');
    return !isPlaceholderRun(runId);
  }).map(row => {
    const cells = row.cells;
    if (cells.length !== 10) throw new Error('Unexpected metric column count');
    const sourceRun = cells[1].title || cells[1].text;
    const { name, configuration } = normalizeRunName(sourceRun);
    const scoreCount = next.problems.reduce((count, problem) => count + problem.weights.length + 1, 0);
    if (row.scores?.length !== scoreCount) throw new Error(`Unexpected score count: ${sourceRun}`);
    let offset = 0;
    const problems = next.problems.map(problem => {
      const subtasks = problem.weights.map(weight => parseScoreBox(row.scores[offset++], weight, false));
      return { id: problem.id, ...parseScoreBox(row.scores[offset++], problem.maxScore, true), subtasks, submissions: [] };
    });
    const judged = cells[4].text.match(/(\d+(?:\.\d+)?)%/);
    return {
      id: sourceRun.toLowerCase().replaceAll('_', '-'), name, configuration, sourceRun,
      description: configuration ? `Sampling · ${configuration}` : 'Sampling',
      contextTokens: parseMetric(cells[2], { tokens: true }), k: parseMetric(cells[3]),
      status: judged ? `${Number(judged[1])}% judged` : 'Not reported',
      scoreKind: 'sampled', score: parseMetric(cells[5]), expectedScore: null,
      scoreAt1: parseMetric(cells[6]), passAtK: parseMetric(cells[7]), passAt1: parseMetric(cells[8]),
      avgThinkTokens: parseMetric(cells[9], { tokens: true }),
      sourceNote: `IOI 2026 subtask scoreboard snapshot captured ${snapshot.capturedAt}. Score is the source Score@k; Score@1 is its average single-attempt score. Subtask and problem averages retain the source tooltip precision. Token counts retain the source display precision (K = 1,000).`,
      problems,
    };
  });
  const arbiter = next.entries.find(entry => entry.id === 'gpt-oss-120b-arbiter');
  if (arbiter) Object.assign(arbiter, { configuration: 'Arbiter', scoreKind: 'observed' });
  next.entries = [...imported, ...(arbiter ? [arbiter] : [])].sort((a, b) => (b.score ?? -1) - (a.score ?? -1));
  next.updatedAt = snapshot.capturedAt;
  next.source = { label: 'Leaderboard data', url: 'https://github.com/horizon-llm/IOI2026/blob/leaderboard/leaderboard/data.json' };
  return validateData(next);
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const [snapshotPath, dataPath = 'leaderboard/data.json'] = process.argv.slice(2);
  if (!snapshotPath) throw new Error('Usage: node scripts/import-reference.mjs <reviewed-snapshot.json> [data.json]');
  const result = importReference(JSON.parse(readFileSync(snapshotPath, 'utf8')), JSON.parse(readFileSync(dataPath, 'utf8')));
  writeFileSync(dataPath, JSON.stringify(result, null, 2) + '\n');
  process.stdout.write(`Imported ${result.entries.length} model runs.\n`);
}
