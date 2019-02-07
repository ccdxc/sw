import { Component, OnInit, OnDestroy, ViewEncapsulation, ViewChild } from '@angular/core';
import { FormArray, FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { Subscription } from 'rxjs';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { SearchService } from '@app/services/generated/search.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { SearchSearchRequest, SearchSearchResponse } from '@sdk/v1/models/generated/search';

import { Table } from 'primeng/table';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { GuidedSearchCriteria, SearchGrammarItem, SearchsuggestionTypes } from '@app/components/search/';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { MatExpansionPanel } from '@angular/material/expansion';
import { AuditEvent } from '@sdk/v1/models/generated/audit';

@Component({
  selector: 'app-auditevents',
  templateUrl: './auditevents.component.html',
  styleUrls: ['./auditevents.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class AuditeventsComponent extends BaseComponent implements OnInit, OnDestroy {
  public static AUDITEVENT: string = 'AuditEvent';

  @ViewChild('auditeventsTable') auditeventsTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;
  @ViewChild('fieldRepeater') fieldRepeater: FieldselectorComponent;
  @ViewChild('auditeventSearchPanel') auditeventSearchExpansionPanel: MatExpansionPanel;

  auditEvents: any[] = [];
  selectedAuditEvent: AuditEvent = null;

  auditEventFieldSelectorOutput: any;

  fieldFormArray = new FormArray([]);
  currentSearchCriteria: string = '';

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/security/ico-app-black.svg',
  };
  cols: any[] = [
    { field: 'user.name', header: 'Who', class: 'auditevents-column-common auditevents-column-who', sortable: true },
    { field: 'meta.mod-time', header: 'Time', class: 'auditevents-column-common auditevents-column-date', sortable: true },
    { field: 'action', header: 'Action', class: 'auditevents-column-common auditevents-column-action', sortable: true },
    { field: 'resource.kind', header: 'Act On (kind)', class: 'auditevents-column-common auditevents-column-act_on', sortable: true },
    { field: 'resource.name', header: 'Act On (name)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false },
    { field: 'stage', header: 'Stage', class: 'auditevents-column-common auditevents-column-stage', sortable: true },
    { field: 'level', header: 'Level', class: 'auditevents-column-common auditevents-column-level', sortable: true },
    { field: 'outcome', header: 'Outcome', class: 'auditevents-column-common auditevents-column-outcome', sortable: true },
    { field: 'client-ips', header: 'Client', class: 'auditevents-column-common auditevents-column-client_ips', sortable: true },
    { field: 'gateway-node', header: 'Service Node', class: 'auditevents-column-common auditevents-column-gateway_node', sortable: true },
    { field: 'service-name', header: 'Service Name', class: 'auditevents-column-common auditevents-column-service_name', sortable: true },
  ];

  subscriptions: Subscription[] = [];

  constructor(
    protected controllerService: ControllerService,
    protected searchService: SearchService,
  ) {
    super(controllerService, null);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AuditeventsComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.populateFieldSelector();
    this.getAllAuditevents();
    this.setToolbarItems();
  }

  private setToolbarItems() {
    this._controllerService.setToolbarData({
      buttons: [
        {
          cssClass: 'global-button-primary auditevents-toolbar-button',
          text: 'Export',
          callback: () => { this.exportTableData(); },
        },
        {
          cssClass: 'global-button-primary auditevents-toolbar-button',
          text: 'Refresh',
          callback: () => { this.refreshData(); },
        },
        /*  This block is experimental. debug to "toCSVJSON"
        {
          cssClass: 'global-button-primary auditevents-toolbar-button',
          text: 'To CSV/JSON',
          callback: () => { this.toCSVJSON(); },
        }
        // */
      ],
      breadcrumb: [{ label: 'Audit Events', url: Utility.getBaseUIUrl() + 'monitoring/auditevents' }]
    });
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'AuditeventsComponent', 'state': Eventtypes.COMPONENT_DESTROY });

  }

  exportTableData() {
    this.auditeventsTable.exportCSV();
  }

  toCSVJSON () {
    const csv = Utility.extractTableContentToCSV(this.auditeventsTable);
    console.log('csv\n', csv);
    const json = Utility.extractTableContentToJSON(this.auditeventsTable);
    console.log('json\n', json);
    const csv2json = Utility.csvToObjectArray(csv);
    console.log('csv2json\n', csv2json);
  }

  populateFieldSelector() {
    const list = [];
    this.fieldFormArray = new FormArray(list);
  }

  getAllAuditevents() {
    const data = {
      query: {
        kinds: [AuditeventsComponent.AUDITEVENT]
      }
    };
    const searchSearchRequest: SearchSearchRequest = new SearchSearchRequest(data, false);
    this._callSearchRESTAPI(searchSearchRequest);
  }

  /**
   * This serves HTML template
   * @param $event
   */
  handleFieldRepeaterData(values) {
    this.auditEventFieldSelectorOutput = values;
  }

  private _callSearchRESTAPI(searchSearchRequest: SearchSearchRequest) {
    const subscription = this.searchService.PostQuery(searchSearchRequest).subscribe(

      response => {
        const data: SearchSearchResponse = response.body as SearchSearchResponse;
        const objects = data.entries;
        if (!objects || objects.length === 0) {
          this._controllerService.invokeInfoToaster('Information', 'No audit-events found. Please change search criteria.');
        } else {
          const entries = [];
          for (let k = 0; k < objects.length; k++) {
            entries.push(objects[k].object); // objects[k] is a SearchEntry object
          }
          this.auditEvents = entries;
          this.lazyRenderWrapper.resizeTable(200);
        }
      },
      this.restErrorHandler('Fail to fetch audit-events')

    );
    this.subscriptions.push(subscription);
  }

  /**
   * This API serves html template
   */
  displayAuditEvent(): string {
    return JSON.stringify(this.selectedAuditEvent, null, 1);
  }

  /**
   * This API serves html template
   */
  onAuditeventsTableRowClick(event, rowData: any) {
    this.selectedAuditEvent = rowData;
    this.auditeventsTable.toggleRow(rowData, event);
    this.lazyRenderWrapper.resizeTable(); // This is necessary to properly show expanded row.
    return false;
  }

  /**
   * Override super's API.
   * This api serves html template
   * @param auditevent
   * @param col
   */
  displayColumn(auditevent, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(auditevent, fields);
    const column = col.field;
    switch (column) {
      case 'client-ips':
        return Array.isArray(value) ? value.join(',') : value;
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  /**
   * Invoke audit search
   */
  onAuditSearch($event) {
    const obj: GuidedSearchCriteria = this.genGuidedSearchCriteria();
    const isEmpty = SearchUtil.isGuidedSearchCriteriaEmpty(obj);
    if (!isEmpty) {
      this.proceedAuditEventsSearch(obj);
      this.currentSearchCriteria = this.buildSearchCriteriaString();
    }
  }

  /**
   * This API is a helper function. It generates GuidedSearchCriteria from field-selector.
   * @param includeKind
   */
  genGuidedSearchCriteria(includeKind: boolean = true): GuidedSearchCriteria {
    const obj = {
      is: [AuditeventsComponent.AUDITEVENT],
      has: (this.fieldRepeater) ? this.fieldRepeater.getValues() : []
    };
    if (!includeKind) {
      delete obj.is;
    }
    return obj;
  }

  /**
   * Invoke REST API to fetch audit-events data.
   * @param guidedSearchCriteria
   */
  proceedAuditEventsSearch(guidedSearchCriteria: GuidedSearchCriteria) {
    if ((!guidedSearchCriteria.has || guidedSearchCriteria.has.length === 0)) {
      this._controllerService.invokeInfoToaster('Infomation', 'Please specify search criteria');
      return;
    }
    const searchInputString = SearchUtil.getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria);

    const grammarList = SearchUtil.parseSearchInputString(searchInputString);
    const payload = this.buildComplexSearchPayload(grammarList, searchInputString);
    const searchSearchRequest = new SearchSearchRequest(payload, false);  // we don't to fill default values. So set the second parameter as false;
    this._callSearchRESTAPI(searchSearchRequest);
  }

  /**
   * This function builds request json for invoking Search API
   * It should examine the current search context to decide the type of search. (by category, kind, label, fields,etc)
   * @param searched
   */
  protected buildComplexSearchPayload(list: any[], searched: string): any {
    const payload = {
      'max-results': 50,
      'query': {
      }
    };
    // We evaluate the has operations last so that we
    // know if the object kind is an event or not.
    const fieldRequirementIndexes = [];
    for (let i = 0; i < list.length; i++) {
      const obj: SearchGrammarItem = list[i];
      switch (obj.type) {
        case SearchsuggestionTypes.OP_IS:
          payload.query['kinds'] = [AuditeventsComponent.AUDITEVENT];
          break;
        case SearchsuggestionTypes.OP_HAS:
          fieldRequirementIndexes.push(i);
          break;
        default:
          console.error(this.getClassName() + 'buildComplexSearchPayload() does not recognize ' + searched);
      }
    }
    fieldRequirementIndexes.forEach((index) => {
      const obj = list[index];
      const isEvent = payload.query['kinds'] != null && payload.query['kinds'].length === 1 && SearchUtil.isKindInSpecialEventList(payload.query['kinds'][0]);
      payload.query['fields'] = {
        'requirements': SearchUtil.buildSearchFieldsLabelsPayloadHelper(obj, true, isEvent)
      };
    });
    return payload;
  }

  /**
   * This serves HTML API. It clear audit-event search and refresh data.
   * @param $event
   */
  onCancelAuditSearch($event) {
    this.refreshData();
  }

  refreshData() {
    this.auditeventSearchExpansionPanel.expanded = false;
    this.getAllAuditevents();
    this._controllerService.invokeInfoToaster('Infomation', 'Cleared search criteria, audit-events refreshed.');
    this.currentSearchCriteria = '';
  }

  /**
   * This is a helper API.  It generates a string representing search criteria.
   */
  buildSearchCriteriaString(): string {
    const guidedSearchCriteria: GuidedSearchCriteria = this.genGuidedSearchCriteria(false);
    return SearchUtil.getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria);
  }



}
