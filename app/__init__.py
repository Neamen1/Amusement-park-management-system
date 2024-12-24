from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager
from app.config import Config, TestConfig

db = SQLAlchemy()
login_manager = LoginManager()

def create_app(test_config=None):
    app = Flask(__name__)
    
    if test_config:
        app.config.from_object(TestConfig)
    else:
        app.config.from_object(Config)

    db.init_app(app)
    login_manager.init_app(app)
    login_manager.login_view = 'auth.login'
    

    # Register routes
    from app.routes import admin_routes, user_routes, public_routes, auth
    app.register_blueprint(admin_routes.bp)
    app.register_blueprint(user_routes.bp)
    app.register_blueprint(public_routes.bp)
    app.register_blueprint(auth.bp)
    
    return app
