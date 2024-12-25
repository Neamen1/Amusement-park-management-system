from flask import Blueprint, render_template, jsonify, request, current_app
from ..models import Attractions, Sessions
from .query_server import query_cpp_server
import requests


bp = Blueprint('public', __name__, url_prefix='/')

@bp.route('', methods=['GET'])
@bp.route('/attractions', methods=['GET'])
def view_attractions():
    search_query = request.args.get('search', '').strip()

    page = request.args.get('page', 1)
    try:
        page = int(page)
    except ValueError:
        page = 1
    
    if search_query:
        # Звернення до сервера на C++ для пошуку
        status, response = query_cpp_server(search_query)
        if status != 200:
            return jsonify({'error': response}), 400
    
        attractions = Attractions.query.filter(Attractions.id.in_(response)).paginate(page=page,per_page=current_app.config["ATTRACTIONS_PER_PAGE"],error_out=False
        ).items
    else:
        attractions = Attractions.query.paginate(
            page=page,
            per_page=current_app.config["ATTRACTIONS_PER_PAGE"],
            error_out=False
        ).items

    #print(current_app.config["ATTRACTIONS_PER_PAGE"])
    return render_template('public/view_attractions.html', 
                           attractions=attractions, 
                           current_page=page, 
                           ATTRACTIONS_PER_PAGE=current_app.config["ATTRACTIONS_PER_PAGE"])

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
