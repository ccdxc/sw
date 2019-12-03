import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewChild, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MonitoringEventPolicy, IMonitoringEventPolicy, IApiStatus, } from '@sdk/v1/models/generated/monitoring';
import { SyslogComponent } from '@app/components/shared/syslog/syslog.component';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';

@Component({
  selector: 'app-neweventpolicy',
  templateUrl: './neweventpolicy.component.html',
  styleUrls: ['./neweventpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NeweventpolicyComponent extends CreationForm<IMonitoringEventPolicy, MonitoringEventPolicy> implements OnInit, AfterViewInit {
  @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService, uiconfigsService, MonitoringEventPolicy);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  // Empty Hook
  postNgInit() {}

  // Empty Hook
  isFormValid(): boolean {
    return true;
  }

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    this.oldButtons = currToolbar.buttons;
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary eventpolicy-button',
        text: 'CREATE EVENT POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral eventpolicy-button',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getObjectValues(): IMonitoringEventPolicy {
    const obj = this.newObject.getFormGroupValues();
    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      obj.spec[key] = syslogValues[key];
    });
    return obj;
  }

  createObject(object: IMonitoringEventPolicy) {
    return this._monitoringService.AddEventPolicy(object);
  }

  updateObject(newObject: IMonitoringEventPolicy, oldObject: IMonitoringEventPolicy) {
    return this._monitoringService.UpdateEventPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringEventPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringEventPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

}
