import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { BaseComponent} from '@app/components/base/base.component';
import * as authActions from '@app/core';
import { Store } from '@ngrx/store';
import { Utility } from '../../common/Utility';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { Router } from '@angular/router';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LocalStorageService } from '@app/core';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss'],

  encapsulation: ViewEncapsulation.None
})
export class LoginComponent extends BaseComponent implements OnInit, OnDestroy {
  credentials = { username: '', password: '' };
  successMessage = '';
  errorMessage = '';
  returnUrl: string;
  // Used to ensure we are only sending one request at a
  // time. Otherwise the key we store for the user will be outdated
  // due to the second request.
  loginInProgress: boolean = false;

  docLink: string = Utility.getDocURL();

  constructor(
    private _authService: AuthService,
    protected _controllerService: ControllerService,
    protected uiconfigService: UIConfigsService,
    private router: Router,
    private store$: Store<any>,
    private localStorage: LocalStorageService
  ) {
    super(_controllerService, uiconfigService);
  }

  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
     Utility.getInstance().clearAllVeniceObjectCacheData(); // clear all cached data
    if (this._controllerService.isUserLogin()) {
      this.redirect();
      return;
    }
    const subLocalstorage = this.localStorage.getUserdataObservable().subscribe(
      (data) => {
        if (this._controllerService.isUserLogin()) {
          this.redirect();
          return;
        }
      },
    );
    this.subscriptions.push(subLocalstorage);

    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'LoginComponent', 'state': Eventtypes.COMPONENT_INIT });

    // setting up subscription
    const subLoginFailed = this._controllerService.subscribe(Eventtypes.LOGIN_FAILURE, (payload) => {
      // in auth.effect.ts (VS-302), there is a timer ticking after user clicks
      if (Utility.LOGIN_IDLE_SETTIME_HANDLE) { clearTimeout(Utility.LOGIN_IDLE_SETTIME_HANDLE); }
      this.onLoginFailure(payload);
    });
    this.subscriptions.push(subLoginFailed);

    const subLoginSucceed = this._controllerService.subscribe(Eventtypes.LOGIN_SUCCESS, (payload) => {
      if (Utility.LOGIN_IDLE_SETTIME_HANDLE) { clearTimeout(Utility.LOGIN_IDLE_SETTIME_HANDLE); }
      this.onLoginSuccess(payload);
    });
    this.subscriptions.push(subLoginSucceed);

  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'LoginComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.unsubscribeAll();
  }

  /**
   * This api serves html template
   */
  isLoginInputValided(): boolean {
    return (!Utility.isEmpty(this.credentials.username) && !Utility.isEmpty(this.credentials.password));
  }

  /**
  * This api serves html template
  *
  * We are using anulgar ngRX store/effect. When login, we have the store to dispatch an action
  */
  login(): any {
    if (this.loginInProgress) {
      return;
    }
    this.loginInProgress = true;
    const payload = {
      username: this.credentials.username,
      password: this.credentials.password,
      tenant: Utility.getInstance().getTenant()
    };
    this.store$.dispatch(authActions.login(payload));
  }

  onLoginFailure(errPayload) {
    this.loginInProgress = false;
    this.successMessage = '';
    this.errorMessage = this.getErrorMessage(errPayload);
  }

  onLoginSuccess(payload) {
    // Not setting loginInProgress back to false because we should be getting redirected
    // and this component will be destroyed.
    this.loginInProgress = false;
    this.redirect();
  }

  redirect() {
    const redirect = this._authService.redirectUrl;
    if (redirect && this.uiconfigService.canActivateSubRoute(redirect)) {
      this._controllerService.publish(Eventtypes.FETCH_USER_PERMISSIONS, {'reason': 'Login redirect'});  // VS-1030, we want to refresh user permissions.
      this.router.navigateByUrl(redirect);
    } else {
      this.uiconfigService.navigateToHomepage();
    }
  }

  /**
   * Get login error message
   * @param errPayload
   */
  getErrorMessage(errPayload: any): string {
    const msgFailtoLogin = 'Failed to login! ';
    const msgConsultAdmin = 'Please consult system administrator';
    if (!errPayload) {
      return msgFailtoLogin + 'for unknown reason. ' + msgConsultAdmin;
    }
    if (errPayload.message && errPayload.message.status === 0) {
      // This handles case where user is not connected in nework and using the browser cached Venice-UI. Give user-friendly message
      return msgFailtoLogin + ' Please refresh browser and ensure you have network connection';
    } else if (errPayload.message && errPayload.message.status === 401) {
      // handle status =401 authentication failure.
      return msgFailtoLogin + ' Incorrect credentials';
    } else if (errPayload.message && errPayload.message.status === 409) {
      // handle status =409 VS-483 enternal user and local user conflict
      return msgFailtoLogin + (errPayload.message.error && errPayload.message.error.message) ? errPayload.message.error.message : errPayload.message.status + ' ' + errPayload.message.statusText;
    } else if (! Utility.isEmpty(errPayload.message) && typeof errPayload.message === 'string') {
      return msgFailtoLogin + ' ' + errPayload.message;
    } else if (! Utility.isEmpty(errPayload.message)) {
      return msgFailtoLogin + ' ' +  JSON.stringify(errPayload.message.error);
    }
    // Stringifying the object may be ugly. We console the error, and display a generic Server error message
    console.error('Login component received error: ' + errPayload);
    return msgFailtoLogin + 'Server error. ' + msgConsultAdmin;
  }
}
