import { AfterViewInit, Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation, ElementRef, Renderer2 } from '@angular/core';
import { FormControl } from '@angular/forms';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { Table } from 'primeng/table';
import { IPUtility } from '@app/common/IPUtility';
import { SecurityService } from '@app/services/generated/security.service';
import { ISecuritySGPolicy, IApiStatus, ISecuritySGRule, SecuritySGPolicy, SecuritySGRule } from '@sdk/v1/models/generated/security';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ArrayChunkUtility } from '@app/common/ArrayChunkUtility';
import { Utility } from '@app/common/Utility';

class SecuritySGRuleWrapper {
  order: number;
  rule: ISecuritySGRule;
}


@Component({
  selector: 'app-sgpolicy',
  templateUrl: './sgpolicy.component.html',
  styleUrls: ['./sgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(window:resize)': 'resizeTable()'
  }
})
export class SgpolicyComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit {
  @ViewChild('sgpolicyTable') sgpolicyTurboTable: Table;
  viewInitComplete: boolean = false;

  cols: any[];
  sgPolicies: ReadonlyArray<ISecuritySGPolicy> = []; // Holds all policy objects, for initial use case, there will only be one object
  sgPoliciesEventUtility: HttpEventUtility;

  // Used for the table - when true there is a loading icon displayed
  loading: boolean = false;
  showAll = false;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy.svg'
  };

  // Current policy that is being displayed
  selectedPolicy: ISecuritySGPolicy;

  // TODO: Update with actual creator
  creator = 'rsikdar';

  ipFormControl: FormControl = new FormControl('', [
  ]);

  portFormControl: FormControl = new FormControl('', [
  ]);

  sgPolicyRulesChunkUtility = new ArrayChunkUtility(false, []);
  // Whether there is new data that isn't displayed in the table
  hasUpdate: boolean = false;
  // Holds the policy rules that are currently displayed in the table
  sgPolicyRulesLazy: ReadonlyArray<SecuritySGRuleWrapper[]> = [];

  constructor(protected _controllerService: ControllerService,
    private elRef: ElementRef,
    protected _securityService: SecurityService
  ) {
    super(_controllerService);
  }

  tableScroll(event) {
    this.sgPolicyRulesLazy = this.sgPolicyRulesChunkUtility.requestChunk(event.first, event.first + event.rows);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getSGPolicies();

    this.cols = [
      { field: 'sourceIPs', header: 'Source IPs' },
      { field: 'destIPs', header: 'Destination IPs' },
      { field: 'action', header: 'Action' },
      { field: 'protocolPort', header: 'Protocol/Ports' },
    ];
    this._controllerService.setToolbarData({

      buttons: [],
      breadcrumb: [{ label: 'Security', url: '' }, { label: 'Security Policy', url: '' }]
    });
  }

  ipSearch(value: any, filter: any): boolean {
    // value = array of data from the current row
    // filter = value from the filter that will be searched in the value-array
    if (filter === undefined || (filter[0] === '' && filter[1] === '')) {
      return true;
    }

    if (value === undefined || value === null || value.length === 0) {
      return false;
    }

    const ip = filter[0];
    const extractedProtocolPort = IPUtility.extractPortFromString(filter[1]);
    const protocol = extractedProtocolPort.protocol;
    const port = extractedProtocolPort.port;

    return IPUtility.filterRuleByIPv4(value, ip) && IPUtility.filterRuleByPort(value, protocol, port);
  }

  ngAfterViewInit() {
    // Adding our custom filter
    this.sgpolicyTurboTable.filterConstraints['ipSearch'] = this.ipSearch;
    this.viewInitComplete = true;
    // Need to put into next cycle to prevent primeNG overriding
    this.resizeTable(0);
  }

  /**
   * PrimeNG is currently not height responsive when using virtual scroll
   * It's also calculating it's initial height before Flex Layout has time to take effect
   * Whenever the page resizes we manually calculate and set the height.
   * 
   * Delay is the ms to wait before calculating the new dimensions
   */
  resizeTable(delay: number) {
    // Set table to be container minus header
    setTimeout(() => {
      const $ = Utility.getJQuery();
      const containerHeight = $('.sgpolicy-widget').outerHeight();
      const headerHeight = $('.ui-table-caption').outerHeight();
      const tableBodyHeader = $('.ui-table-scrollable-header').outerHeight();
      const newHeight = containerHeight - headerHeight - tableBodyHeader;
      $('.ui-table-scrollable-body').css('max-height', newHeight + 'px');
    }, delay);
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicyComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getSGPolicies() {
    this.loading = true;
    this.sgPoliciesEventUtility = new HttpEventUtility();
    this.sgPolicies = this.sgPoliciesEventUtility.array;
    this._securityService.WatchSGPolicy().subscribe(
      response => {
        const body: any = response.body;
        this.sgPoliciesEventUtility.processEvents(body);
        // only look at the first policy
        if (this.sgPolicies.length > 0) {
          // Set sgpolicyrules
          this.selectedPolicy = new SecuritySGPolicy(this.sgPolicies[0]);
          const rules: SecuritySGRuleWrapper[] = this.addOrderRanking(this.selectedPolicy.spec.rules);
          this.sgPolicyRulesChunkUtility.updateData(rules, false);
          // Auto update array if blank OR if we are at the top of the table (scroll is 0)
          // We skip this if view hasn't been initialized for some reason
          if (this.viewInitComplete) {
            if (this.getTableScroll() === 0) {
              // The last requested chunk should contain the default settings the table wants,
              // as it should have been called in the virtual scroll event on table load
              this.sgPolicyRulesChunkUtility.switchToNewData();
              this.sgPolicyRulesLazy = this.sgPolicyRulesChunkUtility.getLastRequestedChunk();
            } else {
              this.hasUpdate = true;
            }
          }
        }
        this.loading = false;
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    )
  }

  /**
   * Returns the table's current scroll amount
   */
  getTableScroll() {
    return this.elRef.nativeElement.querySelector('.ui-table-scrollable-body').scrollTop
  }

  /**
   * Swithces to use new data and scrolls the table to the top
   */
  resetTableView() {
    this.sgPolicyRulesChunkUtility.switchToNewData();
    // If we are scrolled a lot, the scroll to the top will trigger the table
    // to make a new request. If we are near the top though, it won't trigger so we 
    // must load the new values
    this.sgPolicyRulesLazy = this.sgPolicyRulesChunkUtility.getLastRequestedChunk();
    this.hasUpdate = false;
    this.elRef.nativeElement.querySelector('.ui-table-scrollable-body').scroll(0, 0);
  }

  /**
   * Adds a wrapper object around the rules to store ordering
   * so that they can easily be uniquely identified
   * @param rules 
   */
  addOrderRanking(rules: ISecuritySGRule[]) {
    const retRules = [];
    rules.forEach((rule, index) => {
      retRules.push(
        {
          order: index,
          rule: rule
        }
      )
    });
    return retRules;
  }

  // Used for creating the show all toggle highlighting effect
  computeClass(rowData) {
    if (this.showAll) {
      const ip = this.ipFormControl.value;
      const port = this.portFormControl.value;
      if (this.ipSearch(rowData, [ip, port])) {
        return 'sgpolicy-match';
      }
      return 'sgpolicy-miss';
    }
    return '';
  }

  // Controled by the showall toggle
  toggle(event) {
    this.showAll = event.checked;
    this.filterPolicyRules(event);
  }

  // If the show all toggle is on, we reset the search,
  // Otherwise, we filter the table
  filterPolicyRules(event) {
    if ((this.ipFormControl.value !== '' || this.portFormControl.value !== '') && !this.showAll) {
      const ip = this.ipFormControl.value;
      const port = this.portFormControl.value;
      this.sgpolicyTurboTable.filter([ip, port], 'data', 'ipSearch');
    } else {
      this.sgpolicyTurboTable.filter(null, 'data', 'ipSearch');
    }
  }

}
