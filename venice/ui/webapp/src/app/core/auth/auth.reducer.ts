import { Action } from '@ngrx/store';

export const initialState = {
  isAuthenticated: false
};

export const AUTH_KEY = 'Grpc-Metadata-Csrf-Token';
export const AUTH_BODY = 'AUTH_BODY';
export const AUTH_LOGIN = 'AUTH_LOGIN';
export const AUTH_LOGOUT = 'AUTH_LOGOUT';
export const AUTH_LOGIN_SUCCESS = 'AUTH_LOGIN_SUCCESS';
export const AUTH_LOGIN_FAILURE = 'AUTH_LOGIN_FAILURE';
export const AUTH_LOGOUT_SUCCESS = 'AUTH_LOGOUT_SUCCESS';

export const login = (payload) => ({ type: AUTH_LOGIN, payload: payload });
export const logout = () => ({ type: AUTH_LOGOUT });
export const login_success = (data) => ({ type: AUTH_LOGIN_SUCCESS, payload: data });
export const login_failure = (data) => ({ type: AUTH_LOGIN_FAILURE, payload: data });
export const logout_success = () => ({ type: AUTH_LOGOUT_SUCCESS });

export const selectorAuth = state => state.auth;

export function reducer(state = initialState, action: Action) {
  switch (action.type) {
    case AUTH_LOGIN:
      return Object.assign({}, state, {
        isAuthenticated: false
      });

    case AUTH_LOGOUT:
      return Object.assign({}, state, {
        isAuthenticated: false
      });

    case AUTH_LOGIN_SUCCESS:
      return Object.assign({}, state, {
        isAuthenticated: false
      });

    case AUTH_LOGIN_SUCCESS:
      return Object.assign({}, state, {
        isAuthenticated: false
      });
    case AUTH_LOGOUT_SUCCESS:
      return Object.assign({}, state, {
        isAuthenticated: false
      });

    default:
      return state;
  }
}
