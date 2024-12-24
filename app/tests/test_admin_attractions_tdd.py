import pytest
from app import create_app, db, login_manager

from ..models import Users, Attractions
from app.config import TestConfig

@pytest.fixture()
def app():
    app = create_app(test_config=TestConfig)
    with app.app_context():
        db.create_all()  # Створення таблиць для тестів
        # other setup can go here
        yield app
        db.session.remove()
        db.drop_all()  # Видалення таблиць після тестів

@pytest.fixture
def client(app):
    with app.test_client() as client:
        with app.app_context():
            admin = Users(username='test_admin', 
                          role='admin', 
                          password='admin_pass', 
                          email='admin@test.com')
            db.session.add(admin)
            db.session.commit()
        yield client
        
@login_manager.user_loader
def test_add_attraction_endpoint_exists(client):
    # Авторизуємо адміністратора
    response = client.post('/login', data={'username': 'test_admin', 'password': 'admin_pass'})
    assert response.status_code == 200 # Переконайтеся, що логін успішний

    #Надсилаємо POST-запит на маршрут
    response = client.post('/admin/attractions', data={
        'name': 'Roller Coaster',
        'description': 'High-speed thrill ride',
        'availability': True,
        'max_seats': 20
    })    

    assert response.status_code == 200

def test_add_attraction_creates_entry_in_db(client):
    # Авторизуємо адміністратора
    response = client.post('/login', data={'username': 'test_admin', 'password': 'admin_pass'})
    assert response.status_code == 200 # Переконайтеся, що логін успішний

    # Надсилаємо POST-запит з даними атракціону
    response = client.post('/admin/attractions', data={
        'name': 'Ferris Wheel',
        'description': 'A relaxing ride with a great view',
        'availability': True,
        'max_seats': 40
    })

    # Перевіряємо, чи статус відповіді 200
    assert response.status_code == 200

    # Перевіряємо, чи запис з'явився в базі даних
    
    attraction = Attractions.query.filter_by(name='Ferris Wheel').first()
    assert attraction is not None
    assert attraction.description == 'A relaxing ride with a great view'
    assert attraction.availability is True
    assert attraction.max_seats == 40

def test_add_attraction_invalid_data(client):
    # Авторизуємо адміністратора
    client.post('/login', data={'username': 'test_admin', 'password': 'admin_pass'})

    # Надсилаємо POST-запит з неповними даними
    response = client.post('/admin/attractions', data={
        'name': '',  # Назва атракціону порожня
        'description': 'Test description',
        'availability': True,
        'max_seats': '40'
    })

    # Перевіряємо, що сервер повертає статус-код 400
    assert response.status_code == 400

    # Перевіряємо, чи повідомлення про помилку присутнє
    assert 'error' in response.get_json()
    assert response.get_json()['error'] == 'Name is required.'

        # Надсилаємо POST-запит з неповними даними
    response = client.post('/admin/attractions', data={
        'name': 'test name',  
        'description': 'Test description',
        'availability': True,
        'max_seats': ''   # Кількість місць атракціону порожня або число менше 1
    })

    # Перевіряємо, що сервер повертає статус-код 400
    assert response.status_code == 400

    # Перевіряємо, чи повідомлення про помилку присутнє
    assert 'error' in response.get_json()
    assert response.get_json()['error'] == 'Max seats must be a positive integer.'

