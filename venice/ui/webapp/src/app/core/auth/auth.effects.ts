import { Injectable } from '@angular/core';
import { Actions, Effect } from '@ngrx/effects';
import { Store } from '@ngrx/store';
import { Observable } from 'rxjs/Observable';
import { tap } from 'rxjs/operators/tap';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { UIAction } from '../core.interfaces';
import { LocalStorageService } from '../local-storage/local-storage.service';
import * as authReducer from './auth.reducer';
import { AUTH_KEY, AUTH_LOGIN, AUTH_LOGOUT } from './auth.reducer';




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
    return this.actions$
      .ofType(AUTH_LOGIN)
      .pipe(
        tap(action => {
          this._authService.login(action.payload).subscribe(
            data => {
              const userData = data;
              const isAuthOK = (data) ? true : false;
              if (isAuthOK) {
                this.localStorageService.setItem(AUTH_KEY, { isAuthenticated: true });
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
        }
        )
      );
  }

  protected onLoginFailure(err: any) {
    this.localStorageService.setItem(AUTH_KEY, { isAuthenticated: false });
    this._store.dispatch(authReducer.login_failure(err));
    this._controllerService.publish(Eventtypes.LOGIN_FAILURE, { 'ajax': 'end', 'name': 'login', 'message': err });
  }

  @Effect({ dispatch: false })
  logout(): Observable<UIAction> {
    return this.actions$
      .ofType(AUTH_LOGOUT)
      .pipe(
        tap(action => {
          this.localStorageService.setItem(AUTH_KEY, { isAuthenticated: false });
          this._controllerService.publish(Eventtypes.LOGOUT, {});
          this._store.dispatch(authReducer.logout_success());
        })
      );
  }
}
