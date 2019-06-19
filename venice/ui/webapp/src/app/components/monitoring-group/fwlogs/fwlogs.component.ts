import { Component, OnInit, ViewEncapsulation, ViewChild, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ITelemetry_queryFwlogsQueryResponse, ITelemetry_queryFwlogsQueryList, Telemetry_queryFwlogsQuerySpec, Telemetry_queryFwlog, Telemetry_queryFwlogsQuerySpec_sort_order, ITelemetry_queryFwlog } from '@sdk/v1/models/generated/telemetry_query';
import { TelemetryqueryService } from '@app/services/generated/telemetryquery.service';
import { IPUtility } from '@app/common/IPUtility';
import { LazyLoadEvent, OverlayPanel } from 'primeng/primeng';
import { TableviewAbstract, TablevieweditHTMLComponent } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterService } from '@app/services/generated/cluster.service';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { SecurityService } from '@app/services/generated/security.service';
import { SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { PolicyRuleTuple } from './';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { TimeRange } from '@app/components/shared/timerange/utility';

@Component({
  selector: 'app-fwlogs',
  templateUrl: './fwlogs.component.html',
  styleUrls: ['./fwlogs.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FwlogsComponent extends TableviewAbstract<ITelemetry_queryFwlog, Telemetry_queryFwlog> {
  public static ALLOPTION = 'All';
  @ViewChild(TablevieweditHTMLComponent) tableWrapper: TablevieweditHTMLComponent;
  @ViewChild('ruleDetailsOverlay') overlay: OverlayPanel;
  @ViewChild('logOptions') logOptionsMultiSelect: MultiSelect ;

  dataObjects: ReadonlyArray<Telemetry_queryFwlog> = [];

  isTabComponent = false;
  disableTableWhenRowExpanded = false;

  query: Telemetry_queryFwlogsQuerySpec = new Telemetry_queryFwlogsQuerySpec({ 'sort-order': Telemetry_queryFwlogsQuerySpec_sort_order.Descending }, false);
  actionOptions = Utility.convertEnumToSelectItem(Telemetry_queryFwlogsQuerySpec.propInfo.actions.enum);

  exportFilename: string = 'Venice-fwlogs';
  maxRecords: number = 10000;
  startingSortField: string = 'time';
  startingSortOrder: number = -1;

  naples: ReadonlyArray<ClusterSmartNIC> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;
  macAddrToName: { [key: string]: string; } = {};

  // Used for rule details shows on hover
  currentRule = '';
  policyName = '';
  currentRuleObject = {};
  ruleMap = new Map<string, PolicyRuleTuple>();

  // Holds all policy objects
  sgPolicies: ReadonlyArray<SecuritySGPolicy> = [];
  sgPoliciesEventUtility: HttpEventUtility<SecuritySGPolicy>;

  lastUpdateTime: string = '';

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

  inOverlay: boolean = false;

  selectedTimeRange: TimeRange;

  // Only time is supported as sortable by the backend
  cols: TableCol[] = [
    { field: 'time', header: 'Time', class: 'fwlogs-column', sortable: true, width: 16 },
    { field: 'source', header: 'Source', class: 'fwlogs-column-ip', sortable: true, width: 10 },
    { field: 'destination', header: 'Destination', class: 'fwlogs-column-ip', sortable: true, width: 10 },
    { field: 'protocol', header: 'Protocol', class: 'fwlogs-column-port', sortable: true, width: 7 },
    { field: 'source-port', header: 'Src Port', class: 'fwlogs-column-port', sortable: true, width: 5 },
    { field: 'destination-port', header: 'Dest Port', class: 'fwlogs-column-port', sortable: true, width: 6 },
    { field: 'action', header: 'Action', class: 'fwlogs-column', sortable: true, width: 5 },
    { field: 'reporter-id', header: 'Reporter', class: 'fwlogs-column', sortable: true, width: 7 },
    { field: 'direction', header: 'Direction', class: 'fwlogs-column', sortable: true, width: 7 },
    { field: 'session-id', header: 'Session ID', class: 'fwlogs-column', sortable: true},
    { field: 'session-state', header: 'Session Action', class: 'fwlogs-column', sortable: true},
    { field: 'policy', header: 'Policy Name', class: 'fwlogs-column', sortable: true, width: 8, roleGuard: 'securitysgpolicy_read' },
  ];

  constructor(
    protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    private clusterService: ClusterService,
    protected cdr: ChangeDetectorRef,
    protected telemetryService: TelemetryqueryService,
    protected securityService: SecurityService,
  ) {
    super(controllerService, cdr);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  addAllOption(options: SelectItem[]) {
    let newSelectItems: SelectItem[] = [];
    newSelectItems.push({ label: 'All', value: 'All'});
    newSelectItems = newSelectItems.concat(options);
    return newSelectItems;
  }

  setDefaultToolbar() {
      const buttons = [];
      if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringfwlogpolicy_read)) {
        buttons.push({
          cssClass: 'global-button-primary fwlogs-button',
          text: 'FIREWALL LOG POLICIES',
          callback: () => { this.controllerService.navigate(['/monitoring', 'fwlogs', 'fwlogpolicies']); }
        });
      }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Firewall Logs', url: Utility.getBaseUIUrl() + 'monitoring/fwlogs' }]
    });
  }

  onActionChange(event: any) {
    const values = this.logOptionsMultiSelect.value;
    const index = this.getAllActionIndex(values);  // to check if 'All' is one of the selected actions
    if (values != null && values.length > 1) {
      if (index !== -1 && event.itemValue === FwlogsComponent.ALLOPTION) { // if 'All' is the most recent selected, un-select others
        values[0] = values[index];
        values.splice(1);
      } else if (index !== -1 && event.itemValue !== FwlogsComponent.ALLOPTION) { // if another option selected after 'All', un-select 'All'
        values.splice(index, 1);
      }
    }
  }

  getAllActionIndex(values: any): number {
    return values.findIndex((value: String) => value === FwlogsComponent.ALLOPTION);
  }

  postNgInit() {
    this.getNaples();
    this.query.$formGroup.get('source-ips').setValidators(IPUtility.isValidIPValidator);
    this.query.$formGroup.get('dest-ips').setValidators(IPUtility.isValidIPValidator);
    this.getFwlogs(this.startingSortOrder);
    this.getSGPolicies();
    this.setDefaultToolbar();
    this.actionOptions = this.addAllOption(this.actionOptions);
    this.query.$formGroup.value.actions.push(FwlogsComponent.ALLOPTION);
  }


  setTimeRange(timeRange: TimeRange) {
    // Pushing into next event loop
    setTimeout(() => {
      this.selectedTimeRange = timeRange;
      this.getFwlogs();
    }, 0);
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

  displayPolicyName(data): string {
    return this.ruleMap.get(data['rule-id']) ? this.ruleMap.get(data['rule-id']).policy.meta.name : null;
  }

  getNaplesNameFromReporterID(data: Telemetry_queryFwlog): string {
    if (data == null || data['reporter-id'] == null) {
      return '';
    }
    let name = this.macAddrToName[data['reporter-id']];
    if (name == null || name === '') {
      name = data['reporter-id'];
    }
    return name;
  }

  /**
   * Overriding one in tableviewedit
   */
  exportTableData() {
    const exportMap: CustomExportMap = {};
    exportMap['reporter-id'] = (opts) => {
      return this.getNaplesNameFromReporterID(opts.data);
    };
    exportMap['time'] = (opts) => {
      const dataObj = opts.data as ITelemetry_queryFwlog;
      const time = dataObj.time as any;
      return new PrettyDatePipe('en-US').transform(time, 'ns');
    };
    exportMap['policy'] = (opts) => {
      const dataObj = opts.data as ITelemetry_queryFwlog;
      return this.displayPolicyName(dataObj);
    };
    TableUtility.exportTable(this.cols, this.dataObjects, this.exportFilename, exportMap);
    this.controllerService.invokeInfoToaster('File Exported', this.exportFilename + '.csv');
  }

  clearSearch() {
    this.query = new Telemetry_queryFwlogsQuerySpec({ 'sort-order': Telemetry_queryFwlogsQuerySpec_sort_order.Descending }, false);
    this.getFwlogs();  // after clear search criteria, we want to restore table records.
  }

  onTableSort(event: LazyLoadEvent) {
    this.getFwlogs(event.sortOrder);
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        // mac-address to name map
        this.macAddrToName = {};
        for (const smartnic of this.naples) {
          this.macAddrToName[smartnic.meta.name] = smartnic.spec.id;
        }
      },
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  handleRuleClick() {
    this.controllerService.navigate(['/security', 'sgpolicies', this.policyName]);
  }

  handleOverlayEnter(event) {
    this.inOverlay = true;
  }

  // The icon and overlay generated dont perfectly overlap.
  // This delay allows the user to enter the overlay if they need to copy or select the text
  rowLeave() {
    setTimeout(() => {
      if (!this.inOverlay) {
        this.overlay.hide();
      }
    }, 100);
  }

  handleOverlayLeave(event) {
    this.overlay.hide();
    this.inOverlay = false;
  }

  handleHover(data) {
    this.currentRule = data['rule-id'];
    const mapValue: PolicyRuleTuple = this.ruleMap.get(this.currentRule);
    if (mapValue != null) {
      this.policyName = mapValue.policy.meta.name;
      this.currentRuleObject = mapValue.rule;
    } else {
      this.policyName = null;
      this.currentRuleObject = null;
    }
  }

  getSGPolicies() {
    this.sgPoliciesEventUtility = new HttpEventUtility<SecuritySGPolicy>(SecuritySGPolicy);
    this.sgPolicies = this.sgPoliciesEventUtility.array;
    const subscription = this.securityService.WatchSGPolicy().subscribe(
      response => {
        this.sgPoliciesEventUtility.processEvents(response);
        for (const policy of this.sgPolicies) {
          if (policy.spec != null && policy.spec.rules != null) {
            const rulesLength = policy.spec.rules.length;
            for (let i = 0 ; i < rulesLength ; i++) {
              if (policy.spec.rules[i] != null) {
                const newTuple: PolicyRuleTuple = {
                  policy : policy,
                  rule: policy.spec.rules[i]
                };
                this.ruleMap.set(policy.status['rule-status'][i]['rule-hash'], newTuple);
              }
            }
          }
        }
      },
    );
    this.subscriptions.push(subscription);
  }

  getFwlogs(order = this.tableWrapper.table.sortOrder) {
    if (this.query.$formGroup.invalid) {
      this.controllerService.invokeErrorToaster('Fwlog Search', 'Invalid query');
      return;
    }

    // Remove any invalid query toasters if there are any.
    this.controllerService.removeToaster('Fwlog Search');

    let sortOrder = Telemetry_queryFwlogsQuerySpec_sort_order.Ascending;
    if (order === -1) {
      sortOrder = Telemetry_queryFwlogsQuerySpec_sort_order.Descending;
    }

    const query = new Telemetry_queryFwlogsQuerySpec(null, false);
    const queryVal: any = this.query.getFormGroupValues();
    if (queryVal.actions != null && queryVal.actions.includes(FwlogsComponent.ALLOPTION)) {
      queryVal.actions = null;
    }
    const fields = [
      'source-ips',
      'dest-ips',
      'protocols'
    ];
    fields.forEach(
      (field) => {
        if (typeof queryVal[field] === 'string') {
          query[field] = queryVal[field].split(',').map((e: string) => {
            return e.trim();
          }).filter((e: string) => {
            if (e.length === 0) {
              return false;
            }
            return true;
          });
        }
      }
    );
    const fieldsInt = [
      'source-ports',
      'dest-ports'
    ];
    query.actions = queryVal.actions;

    fieldsInt.forEach(
      (field) => {
        if (typeof queryVal[field] === 'string') {
          query[field] = queryVal[field].split(',')
            .filter((e: string) => {
              if (e.length === 0) {
                return false;
              }
              return true;
            })
            .map((e: string) => {
              return parseInt(e, 10);
            })
            .filter((e) => {
              return !isNaN(e);
            });
        }
      }
    );

    query.pagination.count = this.maxRecords;
    query['sort-order'] = sortOrder;

    if (this.selectedTimeRange != null) {
      query['start-time'] = this.selectedTimeRange.getTime().startTime.toISOString() as any;
      query['end-time'] = this.selectedTimeRange.getTime().endTime.toISOString() as any;
    }

    const queryList: ITelemetry_queryFwlogsQueryList = {
      tenant: Utility.getInstance().getTenant(),
      queries: [
        Utility.TrimDefaultsAndEmptyFields(query)
      ],
    };
    // Get request

    const subscription = this.telemetryService.PostFwlogs(queryList).subscribe(
      (resp) => {
        this.controllerService.removeToaster('Fwlog Search Failed');
        this.lastUpdateTime = new Date().toISOString();
        const body = resp.body as ITelemetry_queryFwlogsQueryResponse;
        const logs = body.results[0].logs;
        if (logs != null) {
          this.dataObjects = logs.map((l) => {
            return new Telemetry_queryFwlog(l);
          });
        } else {
          this.dataObjects = [];
        }
      },
      (error) => {
        this.dataObjects = [];
        this.controllerService.invokeRESTErrorToaster('Fwlog Search Failed', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  keyUpInput(event) {
    if (event.keyCode === SearchUtil.EVENT_KEY_ENTER) {
      this.getFwlogs();
    }
  }

}
