from functools import wraps

from flask import Blueprint, current_app, g, jsonify, make_response
from flask_cors import CORS

from database import SQLiteDB

feature = Blueprint('feature', __name__)
CORS(feature)



@feature.route('/toggle-status/<feature_name>', methods=['GET'])
def feature_toggle_status(feature_name):
    status = get_feature_toggle_status(feature_name)
    return make_response(jsonify(status), 200)

def get_feature_toggle_status(feature_name):
    db: SQLiteDB = current_app.config['db']
    status = db.get_feature({"name":feature_name}, return_key=['enabled'])
    return status

def feature_toggle(feature_name):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            status = get_feature_toggle_status(feature_name)
            if status == 0:
                return jsonify(({"error": f"Feature '{feature_name}' is currently disabled."}), 403)
            return func(*args, **kwargs)
        return wrapper
    return decorator




