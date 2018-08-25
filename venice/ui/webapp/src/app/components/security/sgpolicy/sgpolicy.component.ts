import { AfterViewInit, Component, ElementRef, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { IPUtility } from '@app/common/IPUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import { SecurityService } from '@app/services/generated/security.service';
import { SearchPolicySearchRequest } from '@sdk/v1/models/generated/search';
import { IApiStatus, ISecuritySGPolicy, ISecuritySGRule, SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { Table } from 'primeng/table';
import { SearchUtil } from '@app/components/search/SearchUtil';

/**
 * Component for displaying a security policy and providing IP searching
 * on the rules. 
 * 
 * When there is text entered in any of the search boxes, the
 * search button and the clear search button appear. User can hit enter in
 * any of the boxes to trigger a search or can hit the search button. If they
 * are searching an invalid IP, an error message will show up to the right
 * of the buttons. If the search executes and returns no matches, we display
 * an error message of No Matching Results. Otherwise, the table will scroll
 * to the matching rule and it will be highlighted. 
 * 
 * If the user starts typing in the search box and changes the query, 
 * then the highlighting wil disappear but the user remains at current 
 * scroll position. If the user clicks the clear search button, 
 * the search will clear and they will be brought back to the top of the table.
 * 
 * If new data comes in while we have a search displayed, there are two scenarios
 * 1. User clicks the load new data, and we make a new elastic request and take 
 * them to the matching rule in the new data.
 * 2. User makes a new query but haven't clicked load data. We force switch to
 * the new data and then make the query they requested.
 */
class SecuritySGRuleWrapper {
  order: number;
  rule: ISecuritySGRule;
}

@Component({
  selector: 'app-sgpolicy',
  templateUrl: './sgpolicy.component.html',
  styleUrls: ['./sgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class SgpolicyComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit {
  @ViewChild('sgpolicyTable') sgpolicyTurboTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;
  viewInitComplete: boolean = false;

  subscriptions = [];

  cols: any[];
  // Holds all policy objects, for initial use case, there will only be one object
  sgPolicies: ReadonlyArray<ISecuritySGPolicy> = [];
  sgPoliciesEventUtility: HttpEventUtility;

  // Used for the table - when true there is a loading icon displayed
  loading: boolean = false;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy-black.svg'
  };

  policyIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    svgIcon: 'policy'
  };

  // Current policy that is being displayed
  selectedPolicy: ISecuritySGPolicy;

  // TODO: Update with actual creator
  creator = 'pensando';

  sourceIpFormControl: FormControl = new FormControl('', [
  ]);

  destIpFormControl: FormControl = new FormControl('', [
  ]);

  portFormControl: FormControl = new FormControl('', [
  ]);

  // Holds all the policy rules of the currently selected policy
  sgPolicyRules: ReadonlyArray<SecuritySGRuleWrapper[]> = [];

  // Current filter applied to all the data in the table
  currentSearch = null;
  // Index of the rule to highlight
  selectedRuleIndex: number = null;

  // Error message to display next to IP search
  searchErrorMessage = '';

  // subscription for elastic search queries
  searchSubscription;

  constructor(protected _controllerService: ControllerService,
    private elRef: ElementRef,
    protected securityService: SecurityService,
    protected searchService: SearchService
  ) {
    super(_controllerService);
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

  keyUpInput(event) {
    if (event.keyCode === SearchUtil.EVENT_KEY_ENTER) {
      this.invokePolicySearch();
    } else if (this.currentSearch != null) {
      // If the keystroke changed the search fields
      // to be different than the current search
      // we clear the current search and selected index,
      // but we don't scroll to the top like we do on clearSearch
      const sourceIP = this.sourceIpFormControl.value;
      const destIP = this.destIpFormControl.value;
      const port = this.portFormControl.value;
      if (sourceIP !== this.currentSearch.sourceIP ||
        destIP !== this.currentSearch.destIP ||
        port !== this.currentSearch.port) {
        this.searchErrorMessage = '';
        this.selectedRuleIndex = null;
        this.currentSearch.sourceIP = null;
        this.currentSearch.destIP = null;
        this.currentSearch.port = null;
      }
    } else {
      this.searchErrorMessage = '';
    }
  }

  /**
   * Hooked onto the output emitter of lazyRender component
   * for when the data changes
   */
  dataUpdated() {
    if (this.currentSearch != null) {
      this.invokePolicySearch(this.currentSearch.sourceIP, this.currentSearch.destIP, this.currentSearch.port);
    }
  }

  /**
   * Called by HTML to decide whether to show the search button
   */
  showSearchButton(): boolean {
    const sourceIP = this.sourceIpFormControl.value;
    const destIP = this.destIpFormControl.value;
    const port = this.portFormControl.value;
    return (sourceIP != null && sourceIP.length > 0) ||
      (destIP != null && destIP.length > 0) ||
      (port != null && port.length > 0);
  }

  clearSearch() {
    this.sourceIpFormControl.setValue('');
    this.destIpFormControl.setValue('');
    this.portFormControl.setValue('');
    if (this.selectedRuleIndex != null) {
      this.selectedRuleIndex = null;
      // scroll back to top
      this.lazyRenderWrapper.resetTableView();
    }
  }

  invokePolicySearch(sourceIP = null, destIP = null, port = null) {
    // Read values from form control if not provided
    if (sourceIP == null && destIP == null && port == null) {
      sourceIP = this.sourceIpFormControl.value;
      destIP = this.destIpFormControl.value;
      port = this.portFormControl.value;
    }
    if (!IPUtility.isValidIP(sourceIP) && !IPUtility.isValidIP(destIP)) {
      // Set error states
      this.searchErrorMessage = "Invalid IP"
      return false;
    }
    this.searchErrorMessage = '';

    const req = new SearchPolicySearchRequest();
    req.tenant = Utility.getInstance().getTenant();
    if (port != null && port.trim().length !== 0) {
      req.app = port.trim();
    }
    if (sourceIP != null && sourceIP.trim().length !== 0) {
      req['from-ip-address'] = sourceIP.trim();
    } else {
      req['from-ip-address'] = 'any';
    }
    if (destIP != null && destIP.trim().length !== 0) {
      req['to-ip-address'] = destIP.trim();
    } else {
      req['to-ip-address'] = 'any';
    }


    if (this.searchSubscription != null) {
      // There is another call in transit already, we want to ignore
      // the results of that call
      this.searchSubscription.unsubscribe();
    }
    this.currentSearch = {
      sourceIP: sourceIP,
      destIP: destIP,
      port: port
    }
    this.loading = true;
    // If we are displaying old data, we force update to new data
    if (this.lazyRenderWrapper.hasUpdate) {
      // Current search is set to be the new data
      // When on data update fires from resetting the table view,
      // We will call this function again with the provided search.
      this.lazyRenderWrapper.resetTableView();
      return;
    }
    this.searchSubscription = this.searchService.PostPolicyQuery(req).subscribe(
      (data) => {
        // const body = data.body as ISearchPolicySearchResponse;
        const body = data.body as any;
        if (body.status === "MATCH") {
          if (body.results[this.selectedPolicy.meta.name] == null) {
            this.searchErrorMessage = "No Matching Rule"
          } else {
            this.searchErrorMessage = '';
            this.selectedRuleIndex = body.results[this.selectedPolicy.meta.name].index;
            this.lazyRenderWrapper.scrollToRowNumber(this.selectedRuleIndex);
          }
        } else {
          this.searchErrorMessage = "No Matching Rule"
        }
        this.loading = false;
      },
      (error) => {
        console.error('policy search query failed');
      }
    )

  }

  ngAfterViewInit() {
    this.viewInitComplete = true;
  }


  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicyComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    if (this.searchSubscription != null) {
      this.searchSubscription.unsubscribe();
    }
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getSGPolicies() {
    this.sgPoliciesEventUtility = new HttpEventUtility();
    this.sgPolicies = this.sgPoliciesEventUtility.array;
    const subscription = this.securityService.WatchSGPolicy().subscribe(
      response => {
        const body: any = response.body;
        this.sgPoliciesEventUtility.processEvents(body);
        // only look at the first policy
        if (this.sgPolicies.length > 0) {
          // Set sgpolicyrules
          this.selectedPolicy = new SecuritySGPolicy(this.sgPolicies[0]);
          this.sgPolicyRules = this.addOrderRanking(this.selectedPolicy.spec.rules);
        }
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Returns the table's current scroll amount
   */
  getTableScroll() {
    return this.elRef.nativeElement.querySelector('.ui-table-scrollable-body').scrollTop;
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
      );
    });
    return retRules;
  }
}
