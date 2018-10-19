import { Component, OnInit, ViewChild, ViewEncapsulation, OnDestroy } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { EventsEvent_severity_uihint } from '@sdk/v1/models/generated/events';
import { IApiStatus, SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { Table } from 'primeng/table';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';
import { Subscription } from 'rxjs/Subscription';

@Component({
  selector: 'app-sgpolicies',
  templateUrl: './sgpolicies.component.html',
  styleUrls: ['./sgpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SgpoliciesComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('sgpoliciesTable') sgpoliciesTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  subscriptions: Subscription[] = [];
  severityEnum = EventsEvent_severity_uihint;

  // When true, the table displays a loading symbol
  loading = false;

  // Holds all policy objects
  sgPolicies: ReadonlyArray<SecuritySGPolicy> = [];
  sgPoliciesEventUtility: HttpEventUtility<SecuritySGPolicy>;

  // holds a subset (possibly all) of this.sgPolicies
  // This are the sgPolicies that will be displayed
  filteredSGPolicies: SecuritySGPolicy[] = [];

  // All columns are set as not sortable as it isn't currently supported
  cols: any[] = [
    { field: 'meta.name', header: 'SG Policy Name', class: 'sgpolicies-column-name', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'global-column-date', sortable: false },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'global-column-date', sortable: false },
  ];

  bodyIcon = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy-black.svg'
  };

  tableIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'grid_on'
  };

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getSGPolicies();
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Security', url: '' }, { label: 'Security Group Policies', url: '' }]
    });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  getSGPolicies() {
    this.sgPoliciesEventUtility = new HttpEventUtility<SecuritySGPolicy>(SecuritySGPolicy);
    this.sgPolicies = this.sgPoliciesEventUtility.array;
    const subscription = this.securityService.WatchSGPolicy().subscribe(
      response => {
        const body: any = response.body;
        this.sgPoliciesEventUtility.processEvents(body);
        // we currently don't support filter searching, so we
        // set all the policies to be the filtered set
        this.filteredSGPolicies = this.sgPolicies as any;
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.error('Security service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Security service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
    this.subscriptions.push(subscription);
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

  ngOnDestroy() {
    this.subscriptions.forEach(
      subscription => {
        subscription.unsubscribe();
      }
    );
  }

}
