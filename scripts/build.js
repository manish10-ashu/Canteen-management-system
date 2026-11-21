// Cross-platform build script for the C++ canteen core.
//
//   node scripts/build.js          (or: npm run build:core)
//
// Produces, inside ./core :
//   canteen-core[.exe]  - the CLI bridge used by the Node backend
//   canteen-app[.exe]   - the original interactive console application
//
// Works on Windows (MinGW g++ / clang++) and macOS (clang++ / g++).

const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const srcDir = path.join(root, 'src');
const outDir = path.join(root, 'core');

const isWindows = process.platform === 'win32';
const exeSuffix = isWindows ? '.exe' : '';

// Source files shared by both targets (the two mains are added per target).
const coreSources = [
  'food.cpp',
  'cart.cpp',
  'order.cpp',
  'customer.cpp',
  'admin.cpp',
];

function commandExists(cmd) {
  try {
    const probe = spawnSync(cmd, ['--version'], { stdio: 'ignore' });
    return probe.status === 0;
  } catch (error) {
    return false;
  }
}

function pickCompiler() {
  const candidates = isWindows ? ['g++', 'clang++'] : ['clang++', 'g++'];

  for (const candidate of candidates) {
    if (commandExists(candidate)) {
      return candidate;
    }
  }

  return null;
}

const compiler = pickCompiler();

if (!compiler) {
  console.error(
    'No C++ compiler found. Install MinGW-w64 (Windows) or Xcode Command ' +
    'Line Tools (macOS) so that g++/clang++ is on your PATH.'
  );
  process.exit(1);
}

if (!fs.existsSync(outDir)) {
  fs.mkdirSync(outDir, { recursive: true });
}

const stdFlags = ['-std=c++14', '-O2', '-Wall'];
const platformFlags = isWindows ? ['-static-libgcc', '-static-libstdc++'] : [];

function build(targetName, sources, outputName) {
  const args = [
    ...stdFlags,
    ...platformFlags,
    ...sources.map((file) => path.join(srcDir, file)),
    '-o',
    path.join(outDir, outputName + exeSuffix),
  ];

  console.log(`\n[${targetName}] ${compiler} ${args.join(' ')}`);

  const result = spawnSync(compiler, args, { stdio: 'inherit', cwd: root });

  if (result.status !== 0) {
    console.error(`\nBuild failed for target "${targetName}".`);
    process.exit(result.status || 1);
  }

  console.log(`[${targetName}] built -> ${path.join('core', outputName + exeSuffix)}`);
}

build('core', [...coreSources, 'cli.cpp'], 'canteen-core');
build('app', [...coreSources, 'main.cpp'], 'canteen-app');

console.log('\nAll C++ targets built successfully.');
