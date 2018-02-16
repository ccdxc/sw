import { Component, OnDestroy, OnInit, ViewEncapsulation, ViewChild, ElementRef } from '@angular/core';

import { CommonComponent } from '@app/common.component';
import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';

import {Eventtypes} from '../../enum/eventtypes.enum';
import {Logintypes} from '../../enum/logintypes.enum';
import { Utility } from '../../common/Utility';

import { Store } from '@ngrx/store';
import * as authActions from '@app/core';


@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss']
})
export class LoginComponent  extends CommonComponent implements OnInit, OnDestroy {
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

  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
     this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'LoginComponent', 'state': Eventtypes.COMPONENT_DESTROY });
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
      password: this.credentials.password
    };
    this.store$.dispatch(authActions.login(payload));
  }

   /**
   * Just backup the old code
   *
   */
  loginBK() {
    this.errorMessage = '';
    this._controllerService.publish(Eventtypes.AJAX_START, {'ajax': 'start', 'name': 'login'} );
    const payload = '';

    this.successMessage = 'Signing you in ...';
    this._authService.login(payload)
      .subscribe(
        data => {
          // Publish AJAX-END Event

          const isRESTPassed = Utility.isRESTFailed(data);
          if (isRESTPassed) {
              // process server response
              const isLoginPassed = Utility.isRESTFailed(data);
              if (isLoginPassed) {

                this._controllerService.publish(Eventtypes.LOGIN_SUCCESS, data['response']);
                this._controllerService.publish(Eventtypes.AJAX_END, {'ajax': 'end', 'name': 'login'});
              }else {
                this.errorMessage = 'Failed to login! ' + Utility.getRESTMessage(data);
              }
          }else {
            this.errorMessage = 'Failed to sign in!  Please try again later '  + Utility.getRESTMessage(data);
            this._controllerService.publish(Eventtypes.AJAX_END, {'ajax': 'end', 'name': 'login'});
            this._controllerService.publish(Eventtypes.LOGIN_FAILURE,  {'ajax': 'end', 'name': 'login'});
          }
          this.successMessage = '';

        },
        err => {
          this.successMessage = '';
          this._controllerService.publish(Eventtypes.LOGIN_FAILURE,  {'ajax': 'end', 'name': 'login'});
          this.errorMessage = 'Failed to login! ' + err;
          console.error(err);
        }
      );
  }

}
