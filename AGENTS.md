# AGENTS.md

## Cursor Cloud specific instructions

### Overview

EsoCore is a full-stack industrial IoT platform. The three software services are:

| Service | Directory | Port | Tech | Required |
|---------|-----------|------|------|----------|
| Django REST API | `server/` | 8000 | Python 3.11+, Django 5.2, Poetry | Yes |
| Portal (Nuxt.js) | `portal/` | 3000 | Node 22+, Nuxt 4, pnpm | Yes |
| Website (Nuxt.js) | `website/` | 3001 | Node 22+, Nuxt 4, pnpm | No |

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
  `DJANGO_SUPERUSER_USERNAME=admin DJANGO_SUPERUSER_EMAIL=admin@esocore.local DJANGO_SUPERUSER_PASSWORD=admin123 poetry run python manage.py createsuperuser --noinput`
- **pnpm build scripts warning**: pnpm 10+ blocks native dependency build scripts by default. The portal and website
  show warnings about ignored build scripts for `@parcel/watcher`, `esbuild`, `unrs-resolver`, and `vue-demi`.
  The portal works correctly despite these warnings. The website has a pre-existing issue where pages are not
  discovered (shows Nuxt welcome page instead of actual content).
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

- **Server lint**: `cd server && poetry run ruff check . && poetry run black --check . && poetry run isort --check-only .`
- **Server tests**: `cd server && poetry run python manage.py test`
- **Portal lint**: `cd portal && pnpm run lint`
- **Website lint**: `cd website && pnpm run lint`
- **Root markdown lint**: `pnpm run lint:md` (from repository root)
