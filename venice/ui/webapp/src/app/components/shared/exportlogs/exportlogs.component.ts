import { Component, ViewEncapsulation, Input } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '../tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { SecurityService } from '@app/services/generated/security.service';
import { IMonitoringArchiveQuery, MonitoringArchiveRequest, IMonitoringArchiveRequest, IApiStatus} from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { TimeRange } from '../timerange/utility';

@Component({
  selector: 'app-exportlogs',
  templateUrl: './exportlogs.component.html',
  styleUrls: ['./exportlogs.component.scss'],
  encapsulation: ViewEncapsulation.None,
})

export class ExportLogsComponent extends CreationForm<IMonitoringArchiveRequest, MonitoringArchiveRequest> {

  @Input() archiveQuery: IMonitoringArchiveQuery = {};
  @Input() queryType: string = 'AuditEvent';

  validationMessage: string;
  archiveRequestsEventUtility: HttpEventUtility<MonitoringArchiveRequest>;
  exportedArchiveRequests: ReadonlyArray<MonitoringArchiveRequest>;
  watchQuery = {};
  archiveRequestTimeRange: TimeRange;
  showTimerange: boolean = false;
  displayQuery: boolean = false;
  archiveQueryFormatted: string = '';

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService,
    protected securityService: SecurityService) {
    super(controllerService, uiconfigsService, MonitoringArchiveRequest);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit(): void {
    this.genFormattedJSON();
  }

  genFormattedJSON() {
    if (this.archiveQuery.fields !== undefined && this.archiveQuery.fields !== null && this.archiveQuery.fields.requirements !== undefined && this.archiveQuery.fields.requirements !== null && this.archiveQuery.fields.requirements.length === 0 && this.archiveQuery.labels.requirements.length === 0 && this.archiveQuery.texts.length === 1 && this.archiveQuery.texts[0].text.length === 0) {
      this.archiveQueryFormatted = 'No Query Specified!';
    } else {
      this.archiveQueryFormatted = JSON.stringify(this.archiveQuery, null, 1);
    }
  }

  setToolbar(): void {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary global-button-padding',
        text: 'CREATE ARCHIVE',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral global-button-padding',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this.controllerService.setToolbarData(currToolbar);
  }

  getArchiveTimeRange(timeRange) {
    setTimeout(() => {
      this.archiveRequestTimeRange = timeRange;
    }, 0);
  }

  showArchiveQuery() {
    this.displayQuery = true;
  }

  hideArchiveQuery() {
    this.displayQuery = false;
  }

  createObject() {
    const archiveSearchRequest: MonitoringArchiveRequest = new MonitoringArchiveRequest(null, false);
    const archiveSearchRequestJSON = archiveSearchRequest.getFormGroupValues();
    const archiveFormObj = this.newObject.getFormGroupValues();

    // Send start-time and end-time when user has specified them. User can specify timerange, use default 'Past Month' Setting or query for all by not specifying any timerange
    if (this.showTimerange) {
      if (this.archiveRequestTimeRange.startTime !== null) {
        this.archiveQuery['start-time'] =  this.archiveRequestTimeRange.getTime().startTime.toISOString() as any;
      }
      if (this.archiveRequestTimeRange.endTime !== null) {
        this.archiveQuery['end-time'] = this.archiveRequestTimeRange.getTime().endTime.toISOString() as any;
      }
    }

    archiveSearchRequestJSON.kind = 'ArchiveRequest';
    // archiveSearchRequestJSON.meta.name = startendtime.type + '_' + Utility.s4();
    archiveSearchRequestJSON.meta.name = archiveFormObj.meta.name;
    archiveSearchRequestJSON.spec = {
      'type': this.queryType,
      'query': this.archiveQuery
    };

    return this.monitoringService.AddArchiveRequest(archiveSearchRequestJSON);
  }

  updateObject(newObject: IMonitoringArchiveRequest, oldObject: IMonitoringArchiveRequest): Observable<{body: IMonitoringArchiveRequest | IApiStatus | Error, statusCode: number}> {
    throw new Error('Method not supported');
  }

  generateCreateSuccessMsg(object: IMonitoringArchiveRequest): string {
    return 'Created archive request ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringArchiveRequest): string {
    return 'Updated archive request ' + object.meta.name;
  }

  isFormValid(): boolean {
    this.validationMessage = null;
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.validationMessage = 'Error: Request name is required.';
      return false;
    }
    return true;
  }
}
