from flask_cors import CORS
from flask_sqlalchemy import SQLAlchemy
from flask_jwt_extended import JWTManager

cors = CORS()
alchemy_db = SQLAlchemy()
jwt = JWTManager()
