import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { CommonComponent } from '@app/common.component';
import * as authActions from '@app/core';
import { Store } from '@ngrx/store';
import { Utility } from '../../common/Utility';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';



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



  constructor(
    private _authService: AuthService,
    private _controllerService: ControllerService,
    private store$: Store<any>
  ) {
    super();
  }

  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
    if (this._controllerService.isUserLogin()) {
      this._controllerService.directPageAsUserAlreadyLogin();
    }
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'LoginComponent', 'state': Eventtypes.COMPONENT_INIT });

    // setting up subscription
    this.subscriptions[Eventtypes.LOGIN_FAILURE] = this._controllerService.subscribe(Eventtypes.LOGIN_FAILURE, (payload) => {
      this.onLoginFailure(payload);
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
    const payload = {
      username: this.credentials.username,
      password: this.credentials.password,
      tenant: Utility.getInstance().getTenant()
    };
    this.store$.dispatch(authActions.login(payload));
  }

  onLoginFailure(errPayload) {
    this.successMessage = '';
    this.errorMessage = this.getErrorMessage(errPayload);
  }

  private getErrorMessage(errPayload: any): string {
    return 'Failed to login! ' + errPayload.message.status + ' ' + errPayload.message.statusText;
  }
}
