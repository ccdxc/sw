import { Injectable } from '@angular/core';
import { Actions, Effect, ofType } from '@ngrx/effects';
import { Store } from '@ngrx/store';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { UIAction } from '../core.interfaces';
import { LocalStorageService, LocalStorageEvents } from '../local-storage/local-storage.service';
import * as authReducer from './auth.reducer';
import { AUTH_KEY, AUTH_BODY, AUTH_LOGIN, AUTH_LOGOUT } from './auth.reducer';




/**
 *  This is AuthEffect.
 *  We are using ngRX store/effect for state management.
 *
 *
 */
@Injectable()
export class AuthEffects {
  constructor(
    private actions$: Actions<UIAction>,
    private localStorageService: LocalStorageService,
    private _authService: AuthService,
    private _controllerService: ControllerService,
    private _store: Store<any>
  ) { }


  @Effect({ dispatch: false })
  login(): Observable<UIAction> {
    return this.actions$.pipe(
      ofType(AUTH_LOGIN),
      tap(action => {
        this._authService.login(action.payload).subscribe(
          data => {
            const userData = data;
            const isAuthOK = (data) ? true : false;
            if (isAuthOK) {
              sessionStorage.setItem(AUTH_KEY, userData.headers.get(AUTH_KEY));
              sessionStorage.setItem(AUTH_BODY, JSON.stringify(userData.body));
              this.localStorageService.setItem(LocalStorageEvents.NEW_LOGIN_DATA, sessionStorage);
              this.localStorageService.removeItem(LocalStorageEvents.NEW_LOGIN_DATA);

              this._controllerService.publish(Eventtypes.LOGIN_SUCCESS, userData);
              this._store.dispatch(authReducer.login_success(userData));
            } else {
              this.onLoginFailure('Fail to login');
            }
          },
          err => {
            this.onLoginFailure(err);
          }
        );
      })
    );
  }

  protected onLoginFailure(err: any) {
    sessionStorage.setItem(AUTH_KEY, null);
    sessionStorage.setItem(AUTH_BODY, null);
    this._store.dispatch(authReducer.login_failure(err));
    this._controllerService.publish(Eventtypes.LOGIN_FAILURE, { 'ajax': 'end', 'name': 'login', 'message': err });
  }

  @Effect({ dispatch: false })
  logout(): Observable<UIAction> {
    return this.actions$.pipe(
      ofType(AUTH_LOGOUT),
      tap(action => {
        sessionStorage.clear();
        this.localStorageService.setItem(LocalStorageEvents.LOGOUT_REQUEST, true);
        this.localStorageService.removeItem(LocalStorageEvents.LOGOUT_REQUEST);

        this._store.dispatch(authReducer.logout_success());
      })
    );
  }
}
