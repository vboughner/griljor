import { defineConfig } from 'vitest/config';

export default defineConfig({
  test: {
    onConsoleLog() {
      return false; // suppress during passing tests; still shown on failure
    },
  },
});
