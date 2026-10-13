# AGENTS.md

Instructions for AI coding agents (Claude Code reads this file through `CLAUDE.md`; Codex reads it directly).

## Project conventions

### BOM management

- After editing any `bom.csv` file in `hardware/` (edge, cables, or sensors), run `python3 tools/sync_bom_data.py` so the
  website's public BOM data (`website/public/bom/*.csv`) stays in sync with the source BOM files.
- `hardware/edge/bom.csv` → `website/public/bom/bom-main.csv`
- `hardware/cables/bom.csv` → `website/public/bom/bom-cables.csv`
- `hardware/sensors/<name>/bom.csv` → `website/public/bom/bom-<slug>.csv`

### Markdown

- Maximum line length is 150 characters. When editing Markdown, fix line-length issues while preserving all content.
- Use 2-space indentation for bullet continuation lines.

### README files

- Never use emojis. Keep content professional and clear, with plain-text section headers (no decorative characters).

## Development environment

### Overview

EsoCore is a full-stack industrial IoT platform. The three software services are:

| Service | Directory | Port | Tech | Required |
|---------|-----------|------|------|----------|
| Django REST API | `server/` | 8000 | Python 3.11+, Django 5.2, Poetry | Yes |
| Portal (Nuxt.js) | `portal/` | 3000 | Node 22.19+ (or 24.11+), Nuxt 4, pnpm | Yes |
| Website (Nuxt.js) | `website/` | 3001 | Node 22.19+ (or 24.11+), Nuxt 4, pnpm | No |

See each project's README for standard setup/run/lint/test commands.

### Key caveats

- **Poetry path**: Poetry is installed via pip to `~/.local/bin`. Ensure `PATH` includes `$HOME/.local/bin` before
  running Poetry commands (`export PATH="$HOME/.local/bin:$PATH"`).
- **Server uses SQLite in dev**: No external database needed. Defaults are sufficient for local development.
- **Server .env not auto-loaded**: The Django settings don't auto-load `.env`. Copy `server/.env.example` to
  `server/.env` for reference, but the defaults in `settings.py` work out of the box for development.
- **Portal .env**: Copy `portal/.env.example` to `portal/.env`. The portal connects to the API at
  `http://localhost:8000/api` by default.
- **Server superuser**: Create with

  ```bash
  DJANGO_SUPERUSER_USERNAME=admin DJANGO_SUPERUSER_EMAIL=admin@esocore.local DJANGO_SUPERUSER_PASSWORD=admin123 \
    poetry run python manage.py createsuperuser --noinput
  ```

- **pnpm settings live in `pnpm-workspace.yaml`**: `portal/` and `website/` each keep `overrides` and the `allowBuilds`
  build-script allowlist in their own `pnpm-workspace.yaml`, not in the `pnpm` field of `package.json`. pnpm 11 (which
  Dependabot uses) ignores that field and drops the overrides from the lockfile, so CI's pnpm 10 `--frozen-lockfile`
  install fails. Add new overrides or build approvals to `pnpm-workspace.yaml`, and use `allowBuilds`, not
  `onlyBuiltDependencies`.
- **Portal login**: Use `admin` / `admin123` (or the superuser credentials you created) to log into the portal.
- **WebSocket port conflict**: When running both portal and website dev servers simultaneously, the website will show
  a WebSocket port 24678 conflict error. This is cosmetic and doesn't affect the website's server-side rendering.

### Running services

```bash
# Server (terminal 1)
cd server && export PATH="$HOME/.local/bin:$PATH" && poetry run python manage.py runserver 0.0.0.0:8000

# Portal (terminal 2)
cd portal && pnpm run dev -- --host 0.0.0.0

# Website (terminal 3, optional)
cd website && pnpm run dev -- --host 0.0.0.0 --port 3001
```

### Lint and test commands

These are the CI gates in `.github/workflows/`. Each `*-ci.yml` workflow runs on every pull request to `main`; run the
gates of the part you changed before pushing.

- **Server** (`server-ci.yml`): `cd server && poetry run ruff check . && poetry run black --check . &&
  poetry run isort --check-only .`, then `poetry run python manage.py test`. CI repeats the tests against PostgreSQL 17
  with `DJANGO_SETTINGS_MODULE=esocore_api.test_settings_postgres`.
- **Portal** (`portal-ci.yml`): `cd portal && pnpm install --frozen-lockfile && pnpm lint && pnpm format:check &&
  pnpm typecheck && pnpm build`. The portal has no test script.
- **Website** (`website-ci.yml`): `cd website && pnpm install --frozen-lockfile && pnpm lint && pnpm format:check &&
  pnpm typecheck && pnpm test && pnpm build`.
- **Tools** (`tools-ci.yml`): `python3 -m unittest discover -s tools -v` from the repository root.
- **Root markdown lint** (not in CI): `pnpm install --lockfile=false && pnpm run lint:md` from the repository root,
  which has no lockfile.

`pages.yml` deploys the website to GitHub Pages on every push to `main`.
