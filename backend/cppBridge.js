// ---------------------------------------------------------------------------
//  Bridge between the Express API and the compiled C++ canteen core.
//
//  The core is a stateless CLI: one process per request, JSON on stdout.
//  Because it keeps no files of its own, this module loads the current state
//  from MongoDB, streams it into the core, and writes the returned state back.
//  It contains no canteen business logic (that all lives in C++).
// ---------------------------------------------------------------------------

const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

const { loadState, saveState, isDbReady } = require('./stateStore');

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

function cleanValue(value) {
  return String(value === undefined || value === null ? '' : value).replace(
    /[\r\n]+/g,
    ' '
  );
}

// Flatten the persisted state into the core's "key=value" protocol lines.
function stateToParams(state) {
  const food = state && Array.isArray(state.food) ? state.food : [];
  const customers =
    state && Array.isArray(state.customers) ? state.customers : [];
  const orders = state && Array.isArray(state.orders) ? state.orders : [];

  const params = {
    food: food.map((item) =>
      [
        item.id,
        cleanValue(item.name),
        cleanValue(item.category),
        cleanValue(item.price),
        item.available ? 1 : 0,
        cleanValue(item.emoji),
        cleanValue(item.description),
        cleanValue(item.color),
      ].join('|')
    ),
    customer: customers.map((item) =>
      [item.id, cleanValue(item.name), cleanValue(item.phone)].join('|')
    ),
    order: orders.map((item) =>
      [
        item.id,
        item.customerId,
        cleanValue(item.customerName),
        cleanValue(item.status),
      ].join('|')
    ),
    orderitem: [],
  };

  orders.forEach((order) => {
    (order.items || []).forEach((item) => {
      params.orderitem.push(
        [
          order.id,
          item.id,
          cleanValue(item.name),
          cleanValue(item.price),
          item.quantity,
        ].join('|')
      );
    });
  });

  return params;
}

function mergeParams(stateParams, commandParams) {
  const merged = { ...stateParams };

  Object.keys(commandParams || {}).forEach((key) => {
    const value = commandParams[key];

    if (value === undefined || value === null) {
      return;
    }

    if (merged[key] === undefined) {
      merged[key] = value;
    } else {
      merged[key] = [].concat(merged[key], value);
    }
  });

  return merged;
}

// Run the core once with a fully built stdin payload.
function executeCore(command, input) {
  return new Promise((resolve, reject) => {
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

        try {
          resolve(JSON.parse(trimmed));
        } catch (error) {
          reject(new CoreError('The core returned invalid JSON.', 500));
        }
      });
    });

    // Stream the request into the core and close stdin so it stops reading.
    if (child.stdin) {
      child.stdin.on('error', () => {
        // The core may exit before reading; the 'close'/'error' handlers
        // above already report the real reason.
      });

      if (input) {
        child.stdin.write(input);
      }

      child.stdin.end();
    }
  });
}

async function runOnce(command, params) {
  if (!isCoreAvailable()) {
    throw new CoreError(
      'The C++ core is not built. Run "npm run build:core" and try again.',
      503
    );
  }

  if (!isDbReady()) {
    throw new CoreError(
      'The database is not connected. Start MongoDB and try again.',
      503
    );
  }

  const state = await loadState();
  const input = buildInput(mergeParams(stateToParams(state), params));

  const parsed = await executeCore(command, input);

  if (!parsed || typeof parsed !== 'object' || !('result' in parsed)) {
    throw new CoreError('The core returned an unexpected response.', 500);
  }

  await saveState(parsed.state);

  return parsed.result;
}

// Serialise core runs so two requests can never read-modify-write the shared
// state at the same time (which would lose an order).
let queue = Promise.resolve();

function runCore(command, params) {
  const run = queue.then(
    () => runOnce(command, params),
    () => runOnce(command, params)
  );

  queue = run.then(
    () => undefined,
    () => undefined
  );

  return run;
}

module.exports = {
  runCore,
  isCoreAvailable,
  CoreError,
  CORE_PATH,
  PROJECT_ROOT,
};
