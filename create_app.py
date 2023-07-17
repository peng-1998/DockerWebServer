from configs import configs
from view import auth, user, admin, machines, containers, jwt_valid, massage
from flask import Flask


def create_app(cfg="default"):
    app = Flask(__name__)
    app.config.from_object(configs[cfg])
    app.config["CORS"].init_app(app)
    app.config["ALCHEMY_DB"].init_app(app)
    app.config["JWT"].init_app(app)

    app.register_blueprint(auth, url_prefix="/api/auth")
    app.register_blueprint(admin, url_prefix="/api/admin")
    app.register_blueprint(containers, url_prefix="/api/containers")
    app.register_blueprint(machines, url_prefix="/api/machines")
    app.register_blueprint(user, url_prefix="/api/user")
    # app.register_blueprint(feature, url_prefix="api/feature")
    app.register_blueprint(jwt_valid)
    app.register_blueprint(massage)

    return app
