import { ChangeDetectorRef, Component, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray } from '@angular/forms';
import { MatExpansionPanel } from '@angular/material/expansion';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { GuidedSearchCriteria, SearchGrammarItem, SearchsuggestionTypes } from '@app/components/search/';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { RowClickEvent, TableCol, TableviewAbstract, TablevieweditHTMLComponent } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import {AuditService} from '@app/services/generated/audit.service';
import {LRUMap} from 'lru_map';
import {AuditEvent, IAuditEvent} from '@sdk/v1/models/generated/audit';
import {SearchSearchQuery_kinds, SearchSearchRequest, SearchSearchRequest_sort_order, SearchSearchResponse } from '@sdk/v1/models/generated/search';
import { LazyLoadEvent } from 'primeng/primeng';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-auditevents',
  templateUrl: './auditevents.component.html',
  styleUrls: ['./auditevents.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class AuditeventsComponent extends TableviewAbstract<IAuditEvent, AuditEvent> {
  public static AUDITEVENT: string = 'AuditEvent';
  @ViewChild(TablevieweditHTMLComponent) tableWrapper: TablevieweditHTMLComponent;
  @ViewChild('fieldRepeater') fieldRepeater: FieldselectorComponent;
  @ViewChild('auditeventSearchPanel') auditeventSearchExpansionPanel: MatExpansionPanel;

  dataObjects: ReadonlyArray<AuditEvent> = [];
  isTabComponent = false;
  disableTableWhenRowExpanded = false;

  totalRecords: number = 0;

  auditEventFieldSelectorOutput: any;

  fieldFormArray = new FormArray([]);
  currentSearchCriteria: string = '';

  exportFilename: string = 'Venice-auditevents';
  numRows: number = 50;

  maxRecords: number = 8000;
  startingSortField: string = 'meta.mod-time';
  startingSortOrder: number = -1;

  loading: boolean = false;
  auditEventDetail:AuditEvent;

  cache = new LRUMap<String, AuditEvent>(Utility.getAuditEventCacheSize());  // cache with limit 10

  lastUpdateTime: string = '';

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/monitoring/ic_audit-black.svg',
  };
  // Backend currently only supports time being sorted
  cols: TableCol[] = [
    { field: 'user.name', header: 'Who', class: 'auditevents-column-common auditevents-column-who', sortable: false, width: 10 },
    { field: 'meta.mod-time', header: 'Time', class: 'auditevents-column-common auditevents-column-date', sortable: true, width: 9 },
    { field: 'action', header: 'Action', class: 'auditevents-column-common auditevents-column-action', sortable: false, width: 9 },
    { field: 'resource.kind', header: 'Act On (kind)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false, width: 9 },
    { field: 'resource.name', header: 'Act On (name)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false, width: 9 },
    { field: 'stage', header: 'Stage', class: 'auditevents-column-common auditevents-column-stage', sortable: false, width: 9 },
    { field: 'level', header: 'Level', class: 'auditevents-column-common auditevents-column-level', sortable: false, width: 9 },
    { field: 'outcome', header: 'Outcome', class: 'auditevents-column-common auditevents-column-outcome', sortable: false, width: 9 },
    { field: 'client-ips', header: 'Client', class: 'auditevents-column-common auditevents-column-client_ips', sortable: false, width: 9 },
    { field: 'gateway-node', header: 'Service Node', class: 'auditevents-column-common auditevents-column-gateway_node', sortable: false, width: 9 },
    { field: 'service-name', header: 'Service Name', class: 'auditevents-column-common auditevents-column-service_name', sortable: false, width: 9 },
  ];

  subscriptions: Subscription[] = [];

  constructor(
    protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected searchService: SearchService,
    protected auditService: AuditService,
  ) {
    super(controllerService, cdr);
  }

  postNgInit() {
    this.populateFieldSelector();
    this.getAuditevents(this.startingSortField, this.startingSortOrder);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefaultToolbar() {
    this.controllerService.setToolbarData({
      buttons: [
        {
          cssClass: 'global-button-primary auditevents-toolbar-button',
          text: 'EXPORT',
          callback: () => { this.exportTableData(); },
        },
        {
          cssClass: 'global-button-primary auditevents-toolbar-button',
          text: 'REFRESH',
          callback: () => { this.getAuditevents(); },
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

  exportTableData() {
    // TODO: Setting hard limit of 8000 for now, Export should be moved to the backend eventually
    Utility.exportTable(this.cols, this.dataObjects, this.exportFilename);
    this.controllerService.invokeInfoToaster('File Exported', this.exportFilename + '.csv');
  }

  toCSVJSON() {
    const csv = Utility.extractTableContentToCSV(this.tableWrapper.table);
    console.log('csv\n', csv);
    const json = Utility.extractTableContentToJSON(this.tableWrapper.table);
    console.log('json\n', json);
    const csv2json = Utility.csvToObjectArray(csv);
    console.log('csv2json\n', csv2json);
  }

  populateFieldSelector() {
    this.fieldFormArray = new FormArray([]);
  }

  getAuditevents(field = this.tableWrapper.table.sortField,
    order = this.tableWrapper.table.sortOrder) {

    this.loading = true;

    let sortOrder = SearchSearchRequest_sort_order.Ascending;
    if (order === -1) {
      sortOrder = SearchSearchRequest_sort_order.Descending;
    }

    let searchSearchRequest: SearchSearchRequest = new SearchSearchRequest(null, false);
    const obj: GuidedSearchCriteria = this.genGuidedSearchCriteria();
    const isEmpty = SearchUtil.isGuidedSearchCriteriaEmpty(obj);
    if (!isEmpty && (obj.has && obj.has.length >= 0)) {
      const searchInputString = SearchUtil.getSearchInputStringFromGuidedSearchCriteria(obj);

      const grammarList = SearchUtil.parseSearchInputString(searchInputString);
      const payload = this.buildComplexSearchPayload(grammarList, searchInputString);
      searchSearchRequest = new SearchSearchRequest(payload, false);  // we don't to fill default values. So set the second parameter as false;
      this.currentSearchCriteria = this.buildSearchCriteriaString();
    }
    searchSearchRequest.query.kinds = [SearchSearchQuery_kinds.AuditEvent];
    searchSearchRequest['sort-by'] = field;
    searchSearchRequest['sort-order'] = sortOrder;
    searchSearchRequest.from = 0;
    searchSearchRequest['max-results'] = this.maxRecords;

    this._callSearchRESTAPI(searchSearchRequest);
  }

  onTableSort(event: LazyLoadEvent) {
    this.getAuditevents(event.sortField, event.sortOrder);
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
        let objects = data.entries;
        if (!objects || objects.length === 0) {
          this.controllerService.invokeInfoToaster('Information', 'No audit-events found. Please change search criteria.');
          objects = [];
        }
        const entries = [];
        for (let k = 0; k < objects.length; k++) {
          entries.push(objects[k].object); // objects[k] is a SearchEntry object
        }
        // Closing row expand if it is open
        if (this.isRowExpanded()) {
          this.onAuditeventsTableRowClick({
            event: null,
            rowData: this.expandedRowData
          });
        }
        this.lastUpdateTime = new Date().toISOString();
        this.dataObjects = entries;
        this.totalRecords = parseInt(data['total-hits'], 10);
        this.loading = false;
      },
      (error) => {
        this.loading = false;
        this.controllerService.invokeRESTErrorToaster('Failed to get audit-events', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * This API serves html template
   */
  displayAuditEvent(): string {
    return JSON.stringify(this.auditEventDetail, null, 1);
  }

  onAuditeventsTableRowClick(event: RowClickEvent) {
    this.tableContainer.table.toggleRow(event.rowData, event.event);
    if (this.expandedRowData === event.rowData) {
      // Click was on the same row
      this.expandedRowData = null;
      this.showRowExpand = false;
    } else {
      this.showRowExpand = true;
      this.expandedRowData = event.rowData;

      // fetch detail audit event data
      const auditEvent = event.rowData;
      this.auditEventDetail = this.cache.get(auditEvent.meta.uuid);  // cache hit
      if(!this.auditEventDetail){
        // cache miss
        this.auditService.GetGetEvent(auditEvent.meta.uuid).subscribe(resp => {
          this.auditEventDetail = resp.body as AuditEvent;  // fetching actual data
          this.cache.set(auditEvent.meta.uuid, this.auditEventDetail);
        }, this.controllerService.restErrorHandler("Failed to get Audit Event"));
      }

      this.expandRowRequest(event.event, event.rowData);
    }
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
    this.currentSearchCriteria = '';
    this.populateFieldSelector();
    this.getAuditevents();
    this.controllerService.invokeInfoToaster('Infomation', 'Cleared search criteria, audit-events refreshed.');
  }

  /**
   * This is a helper API.  It generates a string representing search criteria.
   */
  buildSearchCriteriaString(): string {
    const guidedSearchCriteria: GuidedSearchCriteria = this.genGuidedSearchCriteria(false);
    return SearchUtil.getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria);
  }



}
