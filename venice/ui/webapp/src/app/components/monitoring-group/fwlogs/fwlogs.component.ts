import { Component, OnInit, ViewEncapsulation, ViewChild } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ITelemetry_queryFwlogsQueryResponse, ITelemetry_queryFwlogsQueryList, Telemetry_queryFwlogsQuerySpec, Telemetry_queryFwlog } from '@sdk/v1/models/generated/telemetry_query';
import { TelemetryqueryService } from '@app/services/generated/telemetryquery.service';
import { Table } from 'primeng/table';
import { IPUtility } from '@app/common/IPUtility';

@Component({
  selector: 'app-fwlogs',
  templateUrl: './fwlogs.component.html',
  styleUrls: ['./fwlogs.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FwlogsComponent extends BaseComponent implements OnInit {
  @ViewChild('fwlogsTable') fwlogTable: Table;
  loading = false;
  fwlogs = [];
  query: Telemetry_queryFwlogsQuerySpec = new Telemetry_queryFwlogsQuerySpec(null, false);
  actionOptions = Utility.convertEnumToSelectItem(Telemetry_queryFwlogsQuerySpec.propInfo.actions.enum, ['ALL']);

  bodyIcon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/monitoring/icon-firewall-policy-black.svg'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols = [
    { field: 'src', header: 'Source', class: 'fwlogs-column-ip', sortable: true },
    { field: 'dest', header: 'Destination', class: 'fwlogs-column-ip', sortable: true },
    { field: 'protocol', header: 'Protocol', class: 'fwlogs-column-port', sortable: true },
    { field: 'src-port', header: 'Src Port', class: 'fwlogs-column-port', sortable: true },
    { field: 'dest-port', header: 'Dest Port', class: 'fwlogs-column-port', sortable: true },
    { field: 'action', header: 'Action', class: 'fwlogs-column', sortable: true },
    { field: 'direction', header: 'Direction', class: 'fwlogs-column', sortable: true },
    { field: 'timestamp', header: 'Time', class: 'fwlogs-column', sortable: true },
  ];

  constructor(
    protected controllerService: ControllerService,
    protected telemetryService: TelemetryqueryService,
  ) {
    super(controllerService);
  }

  ngOnInit() {
    this.query.$formGroup.get('source-ips').setValidators(IPUtility.isValidIPValidator)
    this.query.$formGroup.get('dest-ips').setValidators(IPUtility.isValidIPValidator)
    // Setting the toolbar of the app
    this.controllerService.setToolbarData({
      buttons: [
        {
          cssClass: 'global-button-primary fwlogs-button',
          text: 'EXPORT LOGS',
          callback: () => { this.exportTableData(); },
        },
        {
          cssClass: 'global-button-primary fwlogs-button',
          text: 'FIREWALL LOG POLICIES',
          callback: () => { this.controllerService.navigate(['/monitoring', 'fwlogs', 'fwlogpolicies']); }
        },],
      breadcrumb: [{ label: 'Firewall Logs', url: Utility.getBaseUIUrl() + 'monitoring/fwlogs' }]
    });
    this.getFwlogs();
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  exportTableData() {
    this.fwlogTable.exportCSV();
    this.controllerService.invokeInfoToaster('File Exported', this.fwlogTable.exportFilename + '.csv');
  }

  clearSearch() {
    this.query = new Telemetry_queryFwlogsQuerySpec(null, false);
  }

  getFwlogs() {
    if (this.query.$formGroup.invalid) {
      this.controllerService.invokeErrorToaster("fwlog Query", "Invalid query")
      return
    }
    this.loading = true;
    const queryList: ITelemetry_queryFwlogsQueryList = {
      tenant: Utility.getInstance().getTenant(),
      queries: [
        new Telemetry_queryFwlogsQuerySpec()
      ],
    };
    const queryVal: any = this.query.getFormGroupValues();
    const fields = [
      'source-ips',
      'dest-ips',
      'protocols'
    ];
    fields.forEach(
      (field) => {
        if (typeof queryVal[field] === 'string') {
          queryList.queries[0][field] = queryVal[field].split(',').map((e: string) => {
            return e.trim();
          }).filter((e: string) => {
            if (e.length == 0) {
              return false
            }
            return true
          });
        }
      }
    );
    const fieldsInt = [
      'source-ports',
      'dest-ports'
    ];
    queryList.queries[0].actions = queryVal.actions;

    fieldsInt.forEach(
      (field) => {
        if (typeof queryVal[field] === 'string') {
          queryList.queries[0][field] = queryVal[field].split(',').map((e: string) => {
            return parseInt(e, 10);
          });
        }
      }
    );

    // get
    const subscription = this.telemetryService.PostFwlogs(queryList).subscribe(
      (resp) => {
        const body = resp.body as ITelemetry_queryFwlogsQueryResponse;
        const logs = body.results[0].logs;
        if (logs != null) {
          this.fwlogs = logs.map((l) => {
            return new Telemetry_queryFwlog(l);
          });
        } else {
          this.fwlogs = [];
        }
        this.loading = false;
      },
      (error) => {
        this.fwlogs = [];
        this.loading = false;
        this.controllerService.invokeRESTErrorToaster('Fwlog search failed', error);
      }
    );

  }

}
