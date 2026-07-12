# Backend Server

The EsoCore backend is a Django REST Framework application that serves as the central data management and API layer for
the platform. It receives telemetry data from Edge devices, manages device registration and configuration, processes
events and alerts, and exposes REST APIs consumed by the Portal frontend.

---

## Technology Stack

| Component             | Technology                                      | Version |
| --------------------- | ----------------------------------------------- | ------- |
| **Framework**         | Django                                          | 5.2     |
| **REST API**          | Django REST Framework                           | 3.16    |
| **Database**          | PostgreSQL (production) / SQLite (development)  | --      |
| **API Documentation** | drf-spectacular (OpenAPI 3.0 / Swagger / ReDoc) | 0.28    |
| **Authentication**    | API keys (devices), session auth (users)        | --      |
| **Admin Interface**   | django-unfold                                   | 0.65    |
| **Package Manager**   | Poetry                                          | 2.0+    |
| **Python**            | 3.11+                                           | --      |
| **Linting**           | ruff, black, isort                              | --      |

---

## Application Structure

The server code lives in the `server/` directory of the repository:

```
server/
├── esocore_api/              # Django project configuration
│   ├── settings.py            # Database, middleware, installed apps
│   ├── urls.py                # Root URL routing
│   ├── auth_views.py          # Login, logout, current user endpoints
│   ├── dashboard_views.py     # Dashboard summary API
│   ├── branding.py            # Admin panel branding
│   ├── wsgi.py / asgi.py      # WSGI/ASGI entry points
├── devices/                   # Device registry and management
├── assets/                    # Equipment/asset tracking
├── telemetry/                 # Time-series sensor data ingestion
├── events/                    # Event logging and alerting
├── users/                     # User accounts and roles
├── iot/                       # IoT device-facing API (API key auth)
├── templates/admin/           # Custom admin templates
├── manage.py
├── pyproject.toml             # Poetry configuration and dependencies
```

### Django Apps

| App           | Purpose                                                                            |
| ------------- | ---------------------------------------------------------------------------------- |
| **devices**   | Device registry: provisioning, configuration, firmware tracking, fleet management  |
| **assets**    | Equipment and asset models that devices are attached to                            |
| **telemetry** | Time-series data storage for sensor readings; batch ingestion endpoint             |
| **events**    | Structured event logging (connectivity, power, safety, security); alert generation |
| **users**     | User accounts, roles, and permissions                                              |
| **iot**       | Device-facing API endpoints using API key authentication                           |

---

## API Endpoints

### IoT Device API (API Key Authentication)

Devices authenticate using `X-Auth-Key` and `X-Device-Id` headers. These endpoints are designed for machine-to-machine
communication from Edge devices.

| Endpoint                      | Method | Purpose                                 |
| ----------------------------- | ------ | --------------------------------------- |
| `/api/iot/v1/auth/handshake`  | POST   | Device authentication and session setup |
| `/api/iot/v1/telemetry/batch` | POST   | Batch upload of sensor telemetry data   |
| `/api/iot/v1/config`          | GET    | Retrieve device configuration           |
| `/api/iot/v1/ota/check`       | GET    | Check for firmware updates              |
| `/api/iot/v1/ota/report`      | POST   | Report OTA update result                |

### Management API (Session Authentication)

These endpoints are consumed by the Portal frontend and require user session authentication.

| Endpoint                  | Purpose                         |
| ------------------------- | ------------------------------- |
| `/api/devices/`           | Device CRUD, listing, filtering |
| `/api/assets/`            | Asset/equipment management      |
| `/api/telemetry/`         | Query historical telemetry data |
| `/api/events/`            | Browse and filter system events |
| `/api/users/`             | User management                 |
| `/api/auth/login/`        | User login                      |
| `/api/auth/logout/`       | User logout                     |
| `/api/auth/me/`           | Current user profile            |
| `/api/dashboard/summary/` | Aggregated dashboard statistics |

### API Documentation

The server auto-generates interactive API documentation:

| URL            | Format                             |
| -------------- | ---------------------------------- |
| `/api/docs/`   | Swagger UI (interactive)           |
| `/api/schema/` | Raw OpenAPI 3.0 schema (JSON/YAML) |
| `/api/redoc/`  | ReDoc (alternative viewer)         |

---

## Authentication

The server uses two authentication mechanisms:

- **API Key Authentication** (devices): Edge devices include `X-Auth-Key` and `X-Device-Id` headers with every
  request. Keys are provisioned during device manufacturing and can be rotated via OTA. Powered by
  `djangorestframework-api-key`.
- **Session Authentication** (users): The Portal frontend authenticates users via Django's session framework with
  login/logout endpoints. Role-based access control ensures users only see devices and data they are authorized to
  access.

---

## Local Development Setup

### Prerequisites

- Python 3.11 or later
- Poetry 2.0+ (package manager)
- PostgreSQL (optional; SQLite is used by default in development)

### Installation

```bash
cd server

# Install dependencies with Poetry
poetry install

# Run database migrations
poetry run python manage.py migrate

# Create a superuser for admin access
poetry run python manage.py createsuperuser

# Start the development server
poetry run python manage.py runserver
```

The server will be available at `http://localhost:8000`. The admin panel is at `/admin/` and the API documentation at
`/api/docs/`.

### Environment Variables

Key environment variables (set in shell or deployment config). Note: `.env` files are **not** auto-loaded
by the Django settings; set these as real environment variables or add `python-dotenv` loading if needed:

| Variable               | Default                 | Description                       |
| ---------------------- | ----------------------- | --------------------------------- |
| `SECRET_KEY`           | (generated)             | Django secret key                 |
| `DEBUG`                | `True`                  | Enable debug mode                 |
| `DATABASE_URL`         | `sqlite:///db.sqlite3`  | Database connection string        |
| `ALLOWED_HOSTS`        | `localhost,127.0.0.1`   | Comma-separated allowed hostnames |
| `CORS_ALLOWED_ORIGINS` | `http://localhost:3000` | Portal origin for CORS            |

### Running Tests

```bash
poetry run python manage.py test
```

### Code Quality

The project uses ruff for linting, black for formatting, and isort for import sorting:

```bash
poetry run ruff check .
poetry run black .
poetry run isort .
```

Configuration for all tools is in `pyproject.toml`.

---

## Admin Interface

The server includes a customized Django admin panel powered by django-unfold. It provides a modern interface for:

- Managing devices and their configurations
- Viewing telemetry data and event logs
- Managing user accounts and permissions
- Monitoring device fleet status

Access the admin at `/admin/` after creating a superuser account.

---

## Related Documentation

- [API Specification](/docs/api-specification) -- Detailed REST API documentation and data models
- [Cloud Infrastructure](/docs/cloud-infrastructure) -- Deployment architecture and hosting options
- [Portal](/docs/portal) -- The frontend application that consumes these APIs
- [Data Format Specification](/docs/data-format-specification) -- JSON schemas for telemetry data
