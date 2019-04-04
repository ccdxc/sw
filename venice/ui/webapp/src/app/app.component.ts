import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { LogService } from '@app/services/logging/log.service';
import { CommonComponent } from '@app/common.component';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { AuthService, GetUserObjRequest } from './services/generated/auth.service';
import { Eventtypes } from './enum/eventtypes.enum';
import { AUTH_BODY } from '@app/core/auth/auth.reducer';
import { Subscription } from 'rxjs';


/**
 * This is the entry point component of Pensando-Venice Web-Application
 */
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  encapsulation: ViewEncapsulation.None,
  providers: []
})
export class AppComponent extends CommonComponent implements OnInit, OnDestroy {
  subscriptions: Subscription[] = [];

  constructor(
    protected _controllerService: ControllerService,
    protected _logService: LogService,
    protected _authService: AuthService,
  ) {
    super();
  }

  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to instantiate
  */
  ngOnInit() {
    Utility.getInstance().setControllerService(this._controllerService);
    Utility.getInstance().setLogService(this._logService);
    const sub = this._controllerService.subscribe(Eventtypes.FETCH_USER_OBJ, (payload) => {
      this.getUserObj(payload);
    });
    this.subscriptions.push(sub);
  }

  getUserObj(payload: GetUserObjRequest) {
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    this._authService.GetUser(authBody.meta.name).subscribe(payload.success, payload.err);
  }

  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to exit
  */
  ngOnDestroy() {
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
  }

  /**
   * Overide super's API
   * It will return AppComponent
   */
  getClassName(): string {
    return this.constructor.name;
  }

}
