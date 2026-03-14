import { describe, it, expect } from 'vitest';
import { filterText, randomScold } from '../filter';

describe('filterText', () => {
  it('returns original text unmodified when no bad words', () => {
    const result = filterText('hello world');
    expect(result.filtered).toBe('hello world');
    expect(result.triggered).toBe(false);
  });

  it('censors a bad word, preserving first letter', () => {
    const result = filterText('what the hell');
    expect(result.filtered).toBe('what the h***');
    expect(result.triggered).toBe(true);
  });

  it('is case-insensitive', () => {
    const result = filterText('HELL yeah');
    expect(result.triggered).toBe(true);
    expect(result.filtered).not.toContain('HELL');
  });

  it('censors multiple bad words in one string', () => {
    const result = filterText('damn this crap');
    expect(result.triggered).toBe(true);
    expect(result.filtered).toMatch(/^d\*\*\*/);
  });

  it('does not trigger on partial word matches (word boundaries)', () => {
    // "assassin" contains "ass" but should not be triggered by \bass\b
    const result = filterText('assassin');
    expect(result.triggered).toBe(false);
  });

  it('preserves surrounding text', () => {
    const result = filterText('say damn it please');
    expect(result.filtered).toContain('say');
    expect(result.filtered).toContain('please');
  });
});

describe('randomScold', () => {
  it('returns a non-empty string', () => {
    expect(typeof randomScold()).toBe('string');
    expect(randomScold().length).toBeGreaterThan(0);
  });

  it('returns different values across many calls (probabilistic)', () => {
    const results = new Set(Array.from({ length: 50 }, () => randomScold()));
    expect(results.size).toBeGreaterThan(1);
  });
});
