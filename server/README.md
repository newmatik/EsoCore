# EsoCore Server (Django REST API)

Django REST Framework backend for the EsoCore IoT monitoring system.

## Features

- **Device Management**: Register and manage IoT devices with API key authentication
- **Telemetry Ingestion**: High-performance time-series data collection
- **Asset Monitoring**: Track equipment cycles and maintenance schedules
- **Event Processing**: Real-time system events and alert management
- **User Management**: Role-based access control and customizable dashboards
- **RESTful API**: Complete REST API with OpenAPI/Swagger documentation

## Technology Stack

- **Framework**: Django 5.2 with Django REST Framework 3.16
- **Database**: SQLite (development) / PostgreSQL (production)
- **Authentication**: API Key authentication with session support
- **API Documentation**: DRF Spectacular (OpenAPI/Swagger)
- **Deployment**: Docker-ready with production settings

## Quick Start

### Prerequisites

- Python 3.11+
- Poetry (recommended)

### Installation (Poetry)

1. **Navigate to server directory:**

   ```bash
   cd server
   ```

2. **Install dependencies with Poetry:**

   ```bash
   poetry install
   ```

3. **Activate the virtual environment (Poetry 2):**

   ```bash
   poetry env activate
   ```

4. **Run migrations:**

   ```bash
   poetry run python manage.py migrate
   ```

5. **Create superuser:**

   ```bash
   poetry run python manage.py createsuperuser
   ```

6. **Run development server:**

   ```bash
   poetry run python manage.py runserver
   ```

The API will be available at `http://localhost:8000/api/`

## API Endpoints

### Device-Facing Endpoints

- `POST /api/iot/v1/auth/handshake` - Authentication handshake
- `POST /api/iot/v1/telemetry/batch` - Telemetry data ingestion
- `GET /api/iot/v1/config` - Device configuration
- `GET /api/iot/v1/ota/check` - OTA update check
- `POST /api/iot/v1/ota/report` - OTA update status report

### Management Endpoints

- `/api/devices/` - Device management
- `/api/sites/` - Site management
- `/api/assets/` - Asset management
- `/api/events/` - Event management
- `/api/telemetry/` - Telemetry data access

## Authentication

### API Key Authentication (Devices)

Devices authenticate using API keys. Preferred header is `X-Auth-Key` (also accepts `X-API-Key` for compatibility):

```bash
curl -H "X-Device-Id: <uuid>" \
     -H "X-Auth-Key: <device-api-key>" \
     -H "Content-Type: application/json" \
     http://localhost:8000/api/iot/v1/telemetry/batch
```

### Session Authentication (Users)

Users authenticate via Django's session framework for web interface access.

## Configuration

### Environment Variables

Create a `.env` file in the server directory (optional for local dev):

```bash
DEBUG=True
SECRET_KEY=your-secret-key-here
DATABASE_URL=sqlite:///db.sqlite3
ALLOWED_HOSTS=localhost,127.0.0.1
CORS_ALLOWED_ORIGINS=http://localhost:3000,http://127.0.0.1:3000
```

### Branding (Admin Title)

Admin branding is configured in `sentinel_api/branding.py`.

Defaults:

```python
BRAND_NAME = "Newmatik"
PRODUCT_NAME = "EsoCore"
# Admin title becomes: "EsoCore Server"
```

Override via environment variables (no code change needed):

```bash
export BRAND_NAME="Acme"
export PRODUCT_NAME="Edge"
# Optional: override final admin title directly
export ADMIN_TITLE="Acme Edge Server"

poetry run python manage.py runserver
```

The title is used in Unfold's `SITE_TITLE` and the Django admin header/title.

### Database Configuration

For production, use PostgreSQL:

```python
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.postgresql',
        'NAME': 'EsoCore',
        'USER': 'your_user',
        'PASSWORD': 'your_password',
        'HOST': 'localhost',
        'PORT': '5432',
    }
}
```

## Development

### Running Tests

```bash
python manage.py test
```

### Code Formatting

```bash
black .
isort .
```

### API Documentation

Access Swagger UI at: `http://localhost:8000/api/docs/`

## Deployment

### Docker

```bash
docker build -t EsoCore-server .
docker run -p 8000:8000 EsoCore-server
```

### Production Settings

- Set `DEBUG=False`
- Use PostgreSQL database
- Configure proper CORS origins
- Set up proper logging
- Use environment variables for secrets

## Project Structure

```text
server/
├── sentinel_api/          # Main Django project
│   ├── settings.py
│   ├── urls.py
│   └── wsgi.py
├── devices/                   # Device management app
├── assets/                    # Asset management app
├── telemetry/                 # Telemetry data app
├── events/                    # Event processing app
├── users/                     # User management app
├── manage.py
├── pyproject.toml
├── poetry.lock (generated)
├── requirements.txt (optional export)
└── README.md
```

## Contributing

1. Follow Django coding standards
2. Write tests for new features
3. Update API documentation
4. Use meaningful commit messages

## License

Licensed under the Apache License, Version 2.0.
