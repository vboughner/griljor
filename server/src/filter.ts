const BAD_WORDS = [
  'fuck', 'shit', 'bitch', 'asshole', 'bastard', 'damn', 'crap',
  'piss', 'cunt', 'dick', 'cock', 'pussy', 'ass', 'hell',
];

const PATTERNS = BAD_WORDS.map(
  (w) => ({ re: new RegExp(`\\b${w}\\b`, 'gi') })
);

const SCOLDS = [
  'Come on, get a clue, watch your language.',
  "Gimme a break, you needn't swear.",
  "Come on now, what would your mother say?",
  'Please keep it civil in here.',
  "There's no need for that kind of talk.",
];

export function filterText(text: string): { filtered: string; triggered: boolean } {
  let triggered = false;
  let filtered = text;
  for (const { re } of PATTERNS) {
    filtered = filtered.replace(re, (match) => {
      triggered = true;
      return match[0] + '*'.repeat(match.length - 1);
    });
  }
  return { filtered, triggered };
}

export function randomScold(): string {
  return SCOLDS[Math.floor(Math.random() * SCOLDS.length)];
}
