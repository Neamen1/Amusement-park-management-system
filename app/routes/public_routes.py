from flask import Blueprint, render_template, jsonify, request, current_app
from ..models import Attractions, Sessions

bp = Blueprint('public', __name__, url_prefix='/')

@bp.route('', methods=['GET'])
@bp.route('/attractions', methods=['GET'])
def view_attractions():
    page = request.args.get('page', 1)
    try:
        page = int(page)
    except ValueError:
        page = 1
    attractions = Attractions.query.paginate(page=page, per_page=current_app.config["ATTRACTIONS_PER_PAGE"], error_out=False).items
    print(current_app.config["ATTRACTIONS_PER_PAGE"])
    return render_template('public/view_attractions.html', attractions=attractions, current_page=page, ATTRACTIONS_PER_PAGE=current_app.config["ATTRACTIONS_PER_PAGE"])

@bp.route('/sessions', methods=['GET'])
def view_sessions():
    page = request.args.get('page', 1, type=int)
    sessions = Sessions.query.order_by(Sessions.start_time).paginate(
        page=page, 
        per_page=current_app.config["SESSIONS_PER_PAGE"], 
        error_out=False
    )

    return render_template(
        'public/view_sessions.html',
        sessions=sessions.items,
        current_page=page,
        total_pages=sessions.pages,
        SESSIONS_PER_PAGE=current_app.config["SESSIONS_PER_PAGE"]
    )
