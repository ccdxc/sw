import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { LogService } from '@app/services/logging/log.service';
import { BaseComponent} from '@app/components/base/base.component';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from './services/uiconfigs.service';


/**
 * This is the entry point component of Pensando-Venice Web-Application
 */
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  encapsulation: ViewEncapsulation.None,
  providers: []
})
export class AppComponent extends BaseComponent implements OnInit  {
  constructor(
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _logService: LogService,
  ) {
    super(_controllerService, uiconfigsService);
  }

  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to instantiate
  */
  ngOnInit() {
    Utility.getInstance().setControllerService(this._controllerService);
    Utility.getInstance().setLogService(this._logService);
    Utility.getInstance().setUIConfigsService(this.uiconfigsService);
  }

  /**
   * Overide super's API
   * It will return AppComponent
   */
  getClassName(): string {
    return this.constructor.name;
  }

}
