from flask import jsonify, make_response, current_app
from queue import Queue
from flask import request, jsonify, Blueprint
from flask_jwt_extended.exceptions import NoAuthorizationError
from flask_jwt_extended import verify_jwt_in_request

jwt_valid = Blueprint("jwt_valid", __name__)
massage = Blueprint("massage", __name__)


@jwt_valid.before_request
def is_jwt_valid():
    """
    check if the jwt is valid, if not, return 401
    except the login and register request
    """
    if request.endpoint in ["login", "register"]:
        return
    try:
        verify_jwt_in_request()
    except NoAuthorizationError:
        return jsonify({"message": "Invalid token"}, 401)


@massage.route("/api/massage/<user_id>", methods=["GET"])
def get_massage(user_id: int):
    msg_queue: Queue = current_app.config["massage_cache"].get(user_id)
    if msg_queue is None or len(msg_queue) == 0:
        return make_response(jsonify({"message": "no message"}), 404)
    msg = msg_queue.get()
    return make_response(jsonify(msg), 200)
