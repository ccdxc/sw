import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { CommonComponent } from '@app/common.component';
import * as authActions from '@app/core';
import { Store } from '@ngrx/store';
import { Utility } from '../../common/Utility';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { Router } from '@angular/router';
import { UIConfigsService } from '@app/services/uiconfigs.service';



@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss'],

  encapsulation: ViewEncapsulation.None
})
export class LoginComponent extends CommonComponent implements OnInit, OnDestroy {
  credentials = { username: '', password: '' };
  successMessage = '';
  errorMessage = '';
  returnUrl: string;
  // Used to ensure we are only sending one request at a
  // time. Otherwise the key we store for the user will be outdated
  // due to the second request.
  loginInProgress: boolean = false;



  constructor(
    private _authService: AuthService,
    private _controllerService: ControllerService,
    private uiconfigService: UIConfigsService,
    private router: Router,
    private store$: Store<any>
  ) {
    super();
  }

  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
    if (this._controllerService.isUserLogin()) {
      const redirect = this._authService.redirectUrl ? this._authService.redirectUrl : '/dashboard';
      this.router.navigateByUrl(redirect);
    }
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'LoginComponent', 'state': Eventtypes.COMPONENT_INIT });

    // setting up subscription
    this.subscriptions[Eventtypes.LOGIN_FAILURE] = this._controllerService.subscribe(Eventtypes.LOGIN_FAILURE, (payload) => {
      this.onLoginFailure(payload);
    });
    this.subscriptions[Eventtypes.LOGIN_SUCCESS] = this._controllerService.subscribe(Eventtypes.LOGIN_SUCCESS, (payload) => {
      this.onLoginSuccess(payload);
    });
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
    this._controllerService.LoginUserInfo = (payload['body']) ? payload['body'] : payload;
    this._controllerService.LoginUserInfo[Utility.XSRF_NAME] = (payload.headers) ? payload.headers.get(Utility.XSRF_NAME) : '';
    const redirect = this._authService.redirectUrl;
    if (redirect) {
      this.router.navigateByUrl(redirect);
    } else {
      this.uiconfigService.navigateToHomepage();
    }
  }

  private getErrorMessage(errPayload: any): string {
    return 'Failed to login! ' + errPayload.message.status + ' ' + errPayload.message.statusText;
  }
}
