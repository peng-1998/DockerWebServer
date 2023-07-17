import os
from .Extension import cors, alchemy_db, jwt

basedir = os.path.abspath(os.path.dirname(__file__))


class BaseConfig:  # 基本配置类
    SECRET_KEY = os.getenv("SECRET_KEY", "some secret words")
    JWT_SECRECT_KEY = "mycreditentials"
    CORS = cors
    ALCHEMY_DB = alchemy_db
    SQLALCHEMY_DATABASE_URI = "sqlite:///data/test.db"
    JWT = jwt


class DevelopmentConfig(BaseConfig):
    DEBUG = True
    SQLALCHEMY_DATABASE_URI = os.getenv(
        "DEV_DATABASE_URL", "sqlite:///" + os.path.join(basedir, "data-dev.sqlite")
    )


configs = {"development": DevelopmentConfig, "default": BaseConfig}
