// ---------------------------------------------------------------------------
//  Bridge between the Express API and the compiled C++ canteen core.
//
//  The core is a stateless CLI: one process per request, JSON on stdout.
//  This module only knows how to run it safely - it contains no canteen
//  business logic (that all lives in C++).
// ---------------------------------------------------------------------------

const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

// The core resolves "data/*.txt" relative to its working directory, so it must
// always run from the project root on both Windows and macOS.
const PROJECT_ROOT = path.resolve(__dirname, '..');

const CORE_FILENAME =
  process.platform === 'win32' ? 'canteen-core.exe' : 'canteen-core';

const CORE_PATH = path.join(PROJECT_ROOT, 'core', CORE_FILENAME);

const TIMEOUT_MS = 10000;
const MAX_OUTPUT_BYTES = 8 * 1024 * 1024;

class CoreError extends Error {
  constructor(message, statusCode) {
    super(message);
    this.name = 'CoreError';
    this.statusCode = statusCode || 500;
  }
}

function isCoreAvailable() {
  return fs.existsSync(CORE_PATH);
}

// Values may be arrays (repeated keys, e.g. "item") or scalars.
function buildInput(params) {
  const lines = [];

  Object.keys(params || {}).forEach((key) => {
    const value = params[key];

    if (value === undefined || value === null) {
      return;
    }

    const values = Array.isArray(value) ? value : [value];

    values.forEach((entry) => {
      // A value must never be able to inject extra protocol lines.
      const safe = String(entry).replace(/[\r\n]+/g, ' ').trim();
      lines.push(`${key}=${safe}`);
    });
  });

  return lines.length ? lines.join('\n') + '\n' : '';
}

/**
 * Run a core command.
 *
 * @param {string} command  e.g. "list-food", "place-order"
 * @param {object} params   protocol values; array values repeat the key
 * @returns {Promise<object>} parsed JSON produced by the core
 */
function runCore(command, params) {
  return new Promise((resolve, reject) => {
    if (!isCoreAvailable()) {
      reject(
        new CoreError(
          'The C++ core is not built. Run "npm run build:core" and try again.',
          503
        )
      );
      return;
    }

    let child;

    try {
      child = spawn(CORE_PATH, [command], {
        cwd: PROJECT_ROOT,
        windowsHide: true,
      });
    } catch (error) {
      reject(new CoreError('Could not start the core process.', 500));
      return;
    }

    let stdout = '';
    let stderr = '';
    let settled = false;
    let timedOut = false;
    let overflowed = false;

    const timer = setTimeout(() => {
      timedOut = true;
      child.kill();
    }, TIMEOUT_MS);

    function settle(callback) {
      if (settled) {
        return;
      }

      settled = true;
      clearTimeout(timer);
      callback();
    }

    child.stdout.setEncoding('utf8');
    child.stderr.setEncoding('utf8');

    child.stdout.on('data', (chunk) => {
      stdout += chunk;

      if (stdout.length > MAX_OUTPUT_BYTES) {
        overflowed = true;
        child.kill();
      }
    });

    child.stderr.on('data', (chunk) => {
      stderr += chunk;
    });

    child.on('error', (error) => {
      settle(() => {
        if (error.code === 'ENOENT') {
          reject(
            new CoreError(
              'The C++ core executable was not found. Run "npm run build:core".',
              503
            )
          );
          return;
        }

        reject(new CoreError('Core process error: ' + error.message, 500));
      });
    });

    child.on('close', (code) => {
      settle(() => {
        if (timedOut) {
          reject(new CoreError('The core process timed out.', 504));
          return;
        }

        if (overflowed) {
          reject(new CoreError('The core produced too much output.', 500));
          return;
        }

        if (code !== 0) {
          const detail = stderr.trim();
          reject(
            new CoreError(
              'The core process exited with code ' +
                code +
                (detail ? ': ' + detail : '.'),
              500
            )
          );
          return;
        }

        const trimmed = stdout.trim();

        if (!trimmed) {
          reject(new CoreError('The core returned an empty response.', 500));
          return;
        }

        let parsed;

        try {
          parsed = JSON.parse(trimmed);
        } catch (error) {
          reject(new CoreError('The core returned invalid JSON.', 500));
          return;
        }

        resolve(parsed);
      });
    });

    // Stream the request into the core and close stdin so it stops reading.
    if (child.stdin) {
      child.stdin.on('error', () => {
        // The core may exit before reading; the 'close'/'error' handlers
        // above already report the real reason.
      });

      const input = buildInput(params);

      if (input) {
        child.stdin.write(input);
      }

      child.stdin.end();
    }
  });
}

module.exports = {
  runCore,
  isCoreAvailable,
  CoreError,
  CORE_PATH,
  PROJECT_ROOT,
};
