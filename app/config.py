import os

class Config:
    SQLALCHEMY_DATABASE_URI = os.getenv("DATABASE_URL", "postgresql://admin:1221@localhost/amusement_database")
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    SECRET_KEY = os.getenv("SECRET_KEY", "default_secret_key")
    ATTRACTIONS_PER_PAGE = 6
    SESSIONS_PER_PAGE = 10

class TestConfig(Config):
    TESTING = True
    SQLALCHEMY_DATABASE_URI = 'sqlite:///:memory:'  # Тестова база в пам'яті
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    ATTRACTIONS_PER_PAGE = 3
    SESSIONS_PER_PAGE = 10

