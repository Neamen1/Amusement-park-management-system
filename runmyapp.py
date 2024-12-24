from app import create_app, login_manager

app = create_app()

from app.models import Users

@login_manager.user_loader
def load_user(user_id):
    # Flask-Login requires this function to retrieve user objects by ID
    return Users.query.get(int(user_id))


if __name__ == "__main__":
    app.run(debug=True)
