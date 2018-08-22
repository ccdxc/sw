import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { LogService } from '@app/services/logging/log.service';
import { CommonComponent } from '@app/common.component';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';


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

  constructor(
    protected _controllerService: ControllerService,
    protected _logService: LogService,
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
  }

  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to exit
  */
  ngOnDestroy() {
  }

  /**
   * Overide super's API
   * It will return AppComponent
   */
  getClassName(): string {
    return this.constructor.name;
  }

}
