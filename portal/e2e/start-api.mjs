// Starts the Django API for the browser smoke test on a fresh SQLite database:
// migrate, seed_data, the smoke-test fixtures (e2e/fixtures.py), then runserver.
// Playwright runs this as a webServer (playwright.config.ts) and stops it at the end.
//
// E2E_PYTHON is the Python command of the server's environment
// (default "poetry run python"; e.g. "server/.venv/bin/python" relative to the repository).
import { execFileSync, spawn } from 'node:child_process'
import { rmSync, readFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const here = dirname(fileURLToPath(import.meta.url))
const repo = resolve(here, '../..')
const serverDir = join(repo, 'server')
const [python, ...pythonArgs] = (process.env.E2E_PYTHON || 'poetry run python').split(/\s+/)
const pythonCmd = python.includes('/') ? resolve(repo, python) : python

const env = {
  ...process.env,
  DEBUG: 'True',
  SECRET_KEY: process.env.SECRET_KEY || 'portal-smoke-test-only',
  ALLOWED_HOSTS: 'localhost,127.0.0.1',
  CORS_ALLOWED_ORIGINS: process.env.E2E_PORTAL_ORIGIN,
  // Absolute, so the file deleted here is the one Django opens from server/.
  ESOCORE_SQLITE_PATH: resolve(
    process.env.ESOCORE_SQLITE_PATH || join(tmpdir(), 'esocore-portal-e2e.sqlite3')
  ),
}
rmSync(env.ESOCORE_SQLITE_PATH, { force: true })

function manage(args, input) {
  execFileSync(pythonCmd, [...pythonArgs, 'manage.py', ...args], {
    cwd: serverDir,
    env,
    input,
    stdio: [input === undefined ? 'ignore' : 'pipe', 'inherit', 'inherit'],
  })
}

manage(['migrate', '--noinput', '-v', '0'])
manage(['seed_data'])
manage(['shell'], readFileSync(join(here, 'fixtures.py')))

const server = spawn(
  pythonCmd,
  [...pythonArgs, 'manage.py', 'runserver', `127.0.0.1:${process.env.E2E_API_PORT}`, '--noreload'],
  { cwd: serverDir, env, stdio: 'inherit' }
)
for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, () => server.kill(signal))
server.on('exit', code => process.exit(code ?? 0))
