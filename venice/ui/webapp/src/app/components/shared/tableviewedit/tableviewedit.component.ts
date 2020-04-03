import { AfterViewInit, ChangeDetectorRef, Component, EventEmitter, Input, IterableDiffer, OnChanges, OnDestroy, OnInit, Output, Renderer2, SimpleChanges, TemplateRef, ViewChild, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { BaseModel } from '@sdk/v1/models/generated/basemodel/base-model';
import { IApiStatus, FieldsRequirement, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { LazyLoadEvent } from 'primeng/primeng';
import { Table } from 'primeng/table';
import { forkJoin, Observable, Subscription } from 'rxjs';
import { TabcontentInterface } from 'web-app-framework';
import { RowClickEvent, TableCol, CustomExportMap } from '.';
import { LazyrenderComponent } from '../lazyrender/lazyrender.component';
import { TableMenuItem } from '../tableheader/tableheader.component';
import { TableUtility } from './tableutility';
import { LocalSearchRequest, AdvancedSearchComponent } from '../advanced-search/advanced-search.component';
import { SafeStylePipe } from '../Pipes/SafeStyle.pipe';
import { AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';

/**
 * Table view edit component provides an easy way for other pages
 * to create editable tables. This component handles user interactions
 * and state transitions.
 * Current features/behavior:
 *  - on Creation button click, creation template slides down
 *    - Table is covered by an overlay and is not editable
 *    - Switching tabs is disabled
 *  - On row hover, edit buttons show up and row expands to show content
 *    - other data is not editable while it is open
 *    - table becomes frozen and will not update the table even if new data is passed in
 *    - on leaving exit, the table will be updated with new data if the
 *       user is scrolled to the top or will show a "new data" button
 *   - On item deletion, it will prompt a confirm box. The confirm message, as well as the toaster message must be provided by the extending component.
 *
 *
 * Components should extend the TablevieweditAbstract
 * In the constructor they specify whether they are a tab or not,
 * and should either use
 *      @Input() dataObjects
 * or modify the ngOnInit to fetch the data and set dataObjects to be the
 * result.
 * There are also a set of abstract functions that must be implemented.
 *
 * In the HTML, the component should use app-tableviewedit, and pass in the templates
 * they want to use.
 *
 * See EventAlertPolicies and EventPolicies for examples
 */

@Component({
  selector: 'app-tableviewedit',
  templateUrl: './tableviewedit.component.html',
  styleUrls: ['./tableviewedit.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TablevieweditHTMLComponent implements OnInit, AfterViewInit {
  public static MIN_COLUMN_WIDTH: number = 16; // 16px

  @ViewChild('primengTable') table: Table;
  @ViewChild('headerRow') headerRow: any;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  @Input() creatingMode: boolean = false;
  @Input() showRowExpand: boolean = false;
  @Input() disableTableWhenRowExpanded: boolean = true;

  @Input() enableCheckbox: boolean = false;
  @Input() checkboxWidth: number = 40;
  @Input() enableColumnSelect: boolean = true;

  // Lazyrender variables
  @Input() data: any[];
  @Input() runDoCheck: boolean = true;
  @Input() virtualRowHeight: number = 36;
  @Input() isToFetchDataOnSort: boolean = false;
  @Output() lazyLoadEvent: EventEmitter<LazyLoadEvent> = new EventEmitter<LazyLoadEvent>();

  // primeng variables
  @Input() cols: TableCol[] = [];
  @Input() expandedRowData: any;
  @Input() dataKey: string = 'meta.uuid';
  @Input() sortField: string = 'meta.mod-time';
  @Input() sortOrder: number = -1;
  @Input() tableLoading: boolean = false;
  @Input() createTemplate: TemplateRef<any>;
  @Input() captionTemplate: TemplateRef<any>;
  @Input() bodyTemplate: TemplateRef<any>;
  @Input() actionTemplate: TemplateRef<any>;
  @Input() expandTemplate: TemplateRef<any>;
  @Input() compareSelectionBy: string = 'deepEquals';
  @Output() rowClick: EventEmitter<RowClickEvent> = new EventEmitter();

  @Output() rowExpandAnimationComplete: EventEmitter<any> = new EventEmitter();
  @Output() rowSelectedEmitter: EventEmitter<any> = new EventEmitter();
  @Output() rowUnselectedEmitter: EventEmitter<any> = new EventEmitter();

  @Output() operationOnMultiRecordsComplete: EventEmitter<any> = new EventEmitter();

  selectedcolumns: TableCol[];

  actionWidthPx: number = 50;
  actionWidth: number = 5;
  displayedItemCount: number = null;
  pressed: boolean = false;
  headerIndex: number = -1;
  startX: any;
  mouseMovelistener: () => void;
  mouseUpListener: () => void;
  headerRowHandler: any;
  leftCellWidth: any;
  rightCellWidth: any;
  ongoingResize: boolean = false;
  hoveredRowID: string;

  selectedDataObjects: any[] = [];

  constructor(protected renderer: Renderer2) {
  }

  ngOnInit() {
    this.selectedcolumns = this.cols;
  }

  dataUpdated() {
    this.displayedItemCount = this.lazyRenderWrapper.getCurrentDataLength();
  }

  ngAfterViewInit() {
    // Pushing into the next change detection cycle
    setTimeout(() => {
      this.updateWidthPercentages();
      this.displayedItemCount = this.lazyRenderWrapper.getCurrentDataLength();
    }, 0);
  }

  getRowID(rowData: any): string {
    return Utility.getLodash().get(rowData, this.dataKey);
  }

  rowHover(rowData: any) {
    this.hoveredRowID = this.getRowID(rowData);
  }

  resetHover(rowData) {
    // We check if the  row that we are leaving
    // is the row that is saved so that if the rowhover
    // fires for another row before this leave we don't unset it.
    if (this.hoveredRowID === this.getRowID(rowData)) {
      this.hoveredRowID = null;
    }
  }

  updateWidthPercentages() {
    const tableWidth = $(this.headerRow.nativeElement).innerWidth();

    if (tableWidth !== 0) {
      this.actionWidth = this.actionWidthPx / tableWidth; // now converted to percentage
      let sum: number = 0;
      if (this.enableCheckbox) {
        sum += (this.checkboxWidth / tableWidth) * 100;
      }
      if (tableWidth !== 0) {
        const children = this.headerRow.nativeElement.children;
        const startIndex = (this.enableCheckbox) ? 1 : 0;  // check if checkbox is enabled.
        for (let i = startIndex; i < children.length - 1; i++) {
          const newWidth = ($(children[i]).outerWidth() * 100) / tableWidth;
          if (this.selectedcolumns[i - startIndex]) {
            this.selectedcolumns[i - startIndex].width = newWidth;
          }
          sum += newWidth;
        }
        if (this.cols.length === this.selectedcolumns.length) {
          if (this.selectedcolumns[children.length - 1 - startIndex]) {
            this.selectedcolumns[children.length - 1 - startIndex].width = 100 - sum; // TODO: double check (Per Rishabh)
          }
        } else {
          if (this.selectedcolumns.length > 0) {
            const w = parseFloat(String(this.selectedcolumns[this.selectedcolumns.length - 1].width));
            this.selectedcolumns[this.selectedcolumns.length - 1].width = (w + 100 - sum);
          }
        }
      }
    }
  }

  /**
   * This api serves html template
   * @param $event
   */
  onColumnSelectChange($event) {
    const selectedColumns = $event.value;
    if (selectedColumns.length > 0) {
      this.orderSelectedColumns(selectedColumns);
      // this.updateWidthPercentages();
    } else {
      this.selectedcolumns = selectedColumns;
    }
    this.updateWidthPercentages();
  }

  /**
   * This API re-order selected table columns
   * Table columns configuration can be [a, b, c, d, e]
   * Current selected columns is [b, e, c, a]
   * We want to change it to [a, b, c, e]. It follows the original config order.
   */
  orderSelectedColumns(selectedColumns: TableCol[], fieldname: string = 'field') {
    this.selectedcolumns = selectedColumns.sort((a: TableCol, b: TableCol) => {
      const aIndex = this.cols.findIndex((col: TableCol) => col[fieldname] === a[fieldname]);
      const bIndex = this.cols.findIndex((col: TableCol) => col[fieldname] === b[fieldname]);
      return aIndex - bIndex;
    });
  }

  checkIfSelected(rowData: any): string {
    if (this.expandedRowData == null) {
      return '';
    }
    const _ = Utility.getLodash();
    const rowKey = _.get(rowData, this.dataKey);
    const selectedKey = _.get(this.expandedRowData, this.dataKey);
    if (rowKey === selectedKey) {
      return 'tableviewedit-rowhighlight';
    }
    return '';
  }

  /**
   * Column Resizing:
   * Ref : https://medium.com/@rohit22173/creating-re-sizable-columns-in-angular2-d22fbcbe39c9
   *
   * Each table header has a small div at each end (except first and last col) that can be clicked and dragged to resize the adjacent columns.
   *
   * When user clicks on this div we save the indices of the columns being resized.
   * We also store the original width of the two columns and the mouse positions initially.
   *
   * Listeners are created to watch mousemove and mouseup.
   *
   * Mousemove:
   * When the mouse moves we note the displacement from the original position and update the column fxFlex (width) percentage in the table.
   *
   * Mouseup:
   * When the user release the click, we should stop the resizing, hence we reset the pressed flag and stop watching mousemove and mouseup.
  */

  onMouseDown(event, index) {
    this.pressed = true;
    this.headerIndex = index;
    this.startX = event.x;
    this.initResizableColumns();
    this.headerRowHandler = $(event.target).parent().parent().parent();
    const children = this.headerRowHandler.children();
    if (this.enableCheckbox) {
      this.leftCellWidth = $(children[this.headerIndex + 1]).outerWidth();
      this.rightCellWidth = $(children[this.headerIndex + 2]).outerWidth();
    } else {
      this.leftCellWidth = $(children[this.headerIndex]).outerWidth();
      this.rightCellWidth = $(children[this.headerIndex + 1]).outerWidth();
    }
  }

  getNumber(num: number | string): number {
    return typeof num === 'number' ? num : null;
  }

  /**
   * This API compute the column width when resizing.
   */
  setNewWidths(displacement) {
    // check if there is a checkbox. If so, compute the width.
    const tableWidth = $(this.headerRowHandler).width();

    const leftMinWidth = (this.selectedcolumns[this.headerIndex].minColumnWidth) ? this.selectedcolumns[this.headerIndex].minColumnWidth : TablevieweditHTMLComponent.MIN_COLUMN_WIDTH;
    let rightMinWidth = (this.selectedcolumns[this.headerIndex].minColumnWidth) ? this.selectedcolumns[this.headerIndex + 1].minColumnWidth : TablevieweditHTMLComponent.MIN_COLUMN_WIDTH;

    // If last two columns are being resized, we need to worry about actionWidth
    if (this.headerIndex === this.selectedcolumns.length - 2) {
      rightMinWidth += this.actionWidthPx;
    }
    if ((displacement < 0 && this.leftCellWidth + displacement > leftMinWidth) || (displacement > 0 && this.rightCellWidth - displacement > rightMinWidth)) {
      let sum = 0;
      // Incase width is defined using strings, we convert it to percentages first.
      if (typeof this.selectedcolumns[this.headerIndex].width === 'string' || typeof this.selectedcolumns[this.headerIndex + 1].width === 'string') {
        this.updateWidthPercentages();
      }
      sum = this.getNumber(this.selectedcolumns[this.headerIndex].width) + this.getNumber(this.selectedcolumns[this.headerIndex + 1].width);
      this.selectedcolumns[this.headerIndex].width = (this.leftCellWidth + displacement) * 100 / tableWidth;
      this.selectedcolumns[this.headerIndex + 1].width = sum - this.getNumber(this.selectedcolumns[this.headerIndex].width);
      // Rouding results in the widths not adding up to 100%, which causes issues.
      // Hence we maintain the sum of width-percentages of the two columns.
    }
  }

  initResizableColumns() {
    this.mouseMovelistener = this.renderer.listen('body', 'mousemove', (event) => {
      if (this.pressed) {
        const displacement = event.x - this.startX;
        this.setNewWidths(displacement);
      }
    });

    this.mouseUpListener = this.renderer.listen('body', 'mouseup', (event) => {
      if (this.pressed) {
        this.pressed = false;
      }
      if (this.mouseMovelistener) {
        this.mouseMovelistener(); // Stop listening to mousemove when user releases click.
      }
      this.mouseUpListener(); // Stop listening to mouseup when user releases click.
    });
  }

  rowSelected(event) {
    this.rowSelectedEmitter.emit(event);
  }

  rowUnselected(event) {
    this.rowUnselectedEmitter.emit(event);
  }

}

export abstract class TableviewAbstract<I, T extends I> extends BaseComponent implements OnInit, OnDestroy, OnChanges, TabcontentInterface {
  @Output() tableRowExpandClick: EventEmitter<any> = new EventEmitter();

  @ViewChild(TablevieweditHTMLComponent) tableContainer: TablevieweditHTMLComponent;

  subscriptions: Subscription[] = [];

  // Tab Component attributes
  // This can't be picked up through inheritance of the component due to issues with Angular's AOT compiler and abstract classes.
  isActiveTab: boolean = false;
  editMode: EventEmitter<any> = new EventEmitter;

  expandedRowData: any;
  arrayDiffers: IterableDiffer<T>;

  showRowExpand: boolean = false;

  // Whether the toolbar buttons should be enabled
  shouldEnableButtons: boolean = true;

  tableMenuItems: TableMenuItem[] = [
    {
      text: 'Export To CSV',
      onClick: () => {
        this.exportTableDataCSV();
      }
    },
    {
      text: 'Export To JSON',
      onClick: () => {
        this.exportTableDataJSON();
      }
    }
  ];


  // Objects that will be rendered in the table
  abstract dataObjects: ReadonlyArray<T> = [];
  // Whether or not this component is a tab, used for setting toolbar buttons
  abstract isTabComponent: boolean;
  abstract disableTableWhenRowExpanded: boolean;

  abstract exportFilename: string;
  abstract cols: TableCol[];
  // When the table export file is being created,
  // each field will be looked up in the map to determine the value
  // formatting. If an entry doesn't exist, it will apply
  // default formatting
  abstract exportMap: CustomExportMap;

  abstract setDefaultToolbar(): void;

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected uiconifgsService: UIConfigsService) {
    super(controllerService, uiconifgsService);
  }

  ngOnInit() {
    // if not a tab, we will always be the active tab.
    if (!this.isTabComponent) {
      this.isActiveTab = true;
    }
    this.controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': this.getClassName(), 'state':
        Eventtypes.COMPONENT_INIT
    });
    if (this.isActiveTab) {
      this.setDefaultToolbar();
    }
    this.postNgInit();
  }


  // Hook to add extra logic during component initialization
  // Defining as abstract to enforce the idea that ngOnInit shouldn't be overriden unless
  // it's really needed.
  abstract postNgInit(): void;

  isRowExpanded() {
    return this.expandedRowData != null;
  }

  ngOnChanges(changes: SimpleChanges) {
    // We only set the toolbar if we are becoming the active tab,
    if (changes.isActiveTab != null && this.isActiveTab) {
      this.setDefaultToolbar();
    }
  }


  expandRowRequest(event, rowData) {
    if (!this.isRowExpanded()) {
      // Entering edit mode
      // Freezing table so new data doesn't update the table
      if (this.disableTableWhenRowExpanded) {
        this.tableContainer.lazyRenderWrapper.freezeTable();
      }
      this.tableContainer.table.toggleRow(rowData, event);
      this.expandedRowData = rowData;
      if (this.disableTableWhenRowExpanded) {
        this.editMode.emit(true);
        this.shouldEnableButtons = false;
      }
      this.showRowExpand = true;
    }
  }

  closeRowExpand() {
    if (this.isRowExpanded()) {
      this.controllerService.removeToaster(Utility.UPDATE_FAILED_SUMMARY);

      this.showRowExpand = false;
      this.editMode.emit(false);
      this.shouldEnableButtons = true;
      // We don't untoggle the row here, it will happen when rowExpandAnimationComplete
      // is called.
    }
  }

  /**
   * Called when a row expand animation finishes
   * The animation happens when the row expands, and when it collapses
   * If it is expanding, then we are in ediitng mode (set in expandRowRequest).
   * If it is collapsing, then editingMode should be false, (set in closeRowExpand).
   * When it is collapsing, we toggle the row on the turbo table
   *
   * This is because we must wait for the animation to complete before toggling
   * the row on the turbo table for a smooth animation.
   * @param  $event Angular animation end event
   */
  rowExpandAnimationComplete($event) {
    if (!this.showRowExpand) {
      // we are exiting the row expand
      this.tableContainer.table.toggleRow(this.expandedRowData, event);
      this.expandedRowData = null;
      this.tableContainer.lazyRenderWrapper.unfreezeTable();
      // Needed to prevent "ExpressionChangedAfterItHasBeenCheckedError"
      // We force an additional change detection cycle
      this.cdr.detectChanges();
    }
  }

  exportTableDataCSV() {
    TableUtility.exportTableCSV(this.cols, this.dataObjects, this.exportFilename, this.exportMap);
    this.controllerService.invokeInfoToaster('File Exported', this.exportFilename + '.csv');
  }
  exportTableDataJSON() {
    TableUtility.exportTableJSON(this.cols, this.dataObjects, this.exportFilename, this.exportMap);
    this.controllerService.invokeInfoToaster('FileExported', this.exportFilename + '.json');
  }

  /**
   * This is an API hook for child class to override
   */
  ngOnDestroyHook() {
    // do nothing;
  }

  ngOnDestroy() {
    this.ngOnDestroyHook();
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    this.controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': this.getClassName(), 'state':
        Eventtypes.COMPONENT_DESTROY
    });
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [],
    });
  }

  onColumnSelectChange($event) {
    this.tableContainer.onColumnSelectChange($event);
  }

}

export abstract class TablevieweditAbstract<I, T extends I> extends TableviewAbstract<I, T> {
  creatingMode: boolean = false;

  abstract deleteRecord(object: T): Observable<{ body: I | IApiStatus | Error, statusCode: number }>;
  abstract generateDeleteConfirmMsg(object: T): string;
  abstract generateDeleteSuccessMsg(object: T): string;

  /**
   * get the selected rows from p-table widget
   */
  getSelectedDataObjects(): T[] {
    return (this.tableContainer) ? this.tableContainer.selectedDataObjects : [];
  }

  createNewObject() {
    // If a row is expanded, we shouldnt be able to open a create new policy form
    if (!this.isRowExpanded()) {
      this.creatingMode = true;
      this.editMode.emit(true);
    }
  }

  creationFormClose() {
    this.creatingMode = false;
    this.editMode.emit(false);
    this.controllerService.removeToaster(Utility.CREATE_FAILED_SUMMARY);
  }

  expandRowRequest(event, rowData) {
    // If in creation mode, don't allow row expansion
    if (this.creatingMode) {
      return;
    }
    super.expandRowRequest(event, rowData);
  }

  closeRowExpand() {
    // If in creation mode, don't allow row expansion
    if (this.creatingMode) {
      return;
    }
    super.closeRowExpand();
  }

  /** API hook for extended component to act after deleteRecord() is completed */
  postDeleteRecord() { }

  onDeleteRecord(event, object: T) {
    if (event) {
      event.stopPropagation();
    }
    // Should not be able to delete any record while we are editing
    if (this.isRowExpanded()) {
      return;
    }
    this.controllerService.invokeConfirm({
      header: this.generateDeleteConfirmMsg(object),
      message: 'This action cannot be reversed',
      acceptLabel: 'Delete',
      accept: () => {
        const sub = this.deleteRecord(object).subscribe(
          (response) => {
            this.controllerService.invokeSuccessToaster(Utility.DELETE_SUCCESS_SUMMARY, this.generateDeleteSuccessMsg(object));
            this.postDeleteRecord();
          },
          (error) => {
            if (error.body instanceof Error) {
              console.error('Service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
            } else {
              console.error('Service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
            }
            this.controllerService.invokeRESTErrorToaster(Utility.DELETE_FAILED_SUMMARY, error);
          }
        );
        this.subscriptions.push(sub);
      }
    });
  }

  /**
   * Whether there are rows selected in table.
   */
  hasSelectedRows(): boolean {
    return (this.getSelectedDataObjects() && this.getSelectedDataObjects().length > 0);
  }

  /**
   * This api deletes multiple records.
   * 1. build an array observables
   * 2. use forkJoin techniques to invoke REST API
   *
   * @param selectedDataObjects
   * @param summary
   * @param msg
   */
  deleteMultipleRecords(selectedDataObjects: T[], summary: string, partialSuccessSummary: string, msg: string) {
    const observables = [];
    for (let i = 0; selectedDataObjects && i < selectedDataObjects.length; i++) {
      const observable = this.deleteRecord(selectedDataObjects[i]);
      observables.push(observable);
    }
    this.invokeAPIonMultipleRecords(observables, summary, partialSuccessSummary, msg);
  }

  /**
   * This api use forkJoin technique to update multiple records
   * @param observables
   * @param allSuccessSummary
   * @param partialSuccessSummary
   * @param msg
   */
  invokeAPIonMultipleRecords(observables: Observable<T>[], allSuccessSummary: string, partialSuccessSummary: string, msg: string,
    successCallback: () => void = null, errorCallback: (error: any) => void = null
  ) {
    if (observables.length <= 0) {
      return;
    }
    const sub = forkJoin(observables).subscribe(
      (results) => {
        this.tableContainer.operationOnMultiRecordsComplete.emit(results);
        const isAllOK = Utility.isForkjoinResultAllOK(results);
        if (isAllOK) {
          this._controllerService.invokeSuccessToaster(allSuccessSummary, msg);
          // clear the selectedObjects array
          this.clearSelectedDataObjects();
          if (successCallback) {
            successCallback();
          }
        } else {
          const error = Utility.joinErrors(results);
          this._controllerService.invokeRESTErrorToaster(partialSuccessSummary, error);
          if (errorCallback) {
            errorCallback(error);
          }
        }
      },
      (error) => {
        this.tableContainer.operationOnMultiRecordsComplete.emit(error);
        this._controllerService.invokeRESTErrorToaster('Failure', error);
        if (errorCallback) {
          errorCallback(error);
        }
      }
    );
    this.subscriptions.push(sub);
  }

  public clearSelectedDataObjects() {
    if (this.tableContainer && this.tableContainer.selectedDataObjects) {
      this.tableContainer.selectedDataObjects.length = 0;
      this.tableContainer.selectedDataObjects = [];
    }
  }

  /**
   * This API is used in html template. P-table with checkbox enables user to select multiple records. User can delete multiple records.
   * This function asks for user confirmation and invokes the REST API.
   */
  onDeleteSelectedRows($event) {
    const selectedDataObjects = this.getSelectedDataObjects();
    this.controllerService.invokeConfirm({
      header: 'Delete selected ' + selectedDataObjects.length + ' records?',
      message: 'This action cannot be reversed',
      acceptLabel: 'Delete',
      accept: () => {
        const allSuccessSummary = 'Delete';
        const partialSuccessSummary = 'Partially delete';
        const msg = 'Marked selected ' + selectedDataObjects.length + ' deleted.';
        this.invokeDeleteMultipleRecords(allSuccessSummary, partialSuccessSummary, msg);
      }
    });
  }

  /**
   * This api is used in onDeleteSelectedRows(). It finds all selected objects and invokes DELETE API
   * Sub-class should override this function to apply business logic.
   * @param allSuccessSummary
   * @param partialSuccessSummary
   * @param msg
   */
  invokeDeleteMultipleRecords(allSuccessSummary: string, partialSuccessSummary: string, msg: string) {
    const selectedDataObjects = this.getSelectedDataObjects();
    this.deleteMultipleRecords(selectedDataObjects, allSuccessSummary, partialSuccessSummary, msg);
  }

  /**
   * This is API execute table search. See how Hosts and Workload components use it.
   * @param field
   * @param order
   * @param kind
   * @param maxSearchRecords
   * @param advSearchCols
   * @param dataObjects
   * @param advancedSearchComponent
   * @param isShowToasterOnSearchHasResult default is false.
   * @param isShowToasterOnSearchNoResult default is true.
   */
  onSearchDataObjects(field, order, kind: string, maxSearchRecords, advSearchCols: TableCol[], dataObjects, advancedSearchComponent: AdvancedSearchComponent,
    isShowToasterOnSearchHasResult: boolean = false,
    isShowToasterOnSearchNoResult: boolean = true): any[] | ReadonlyArray<any> {
    try {
      const searchSearchRequest = advancedSearchComponent.getSearchRequest(field, order, kind, true, maxSearchRecords);
      const localSearchRequest: LocalSearchRequest = advancedSearchComponent.getLocalSearchRequest(field, order);
      const requirements: FieldsRequirement[] = (searchSearchRequest.query.fields.requirements) ? searchSearchRequest.query.fields.requirements : [];
      const localRequirements: FieldsRequirement[] = (localSearchRequest.query) ? localSearchRequest.query as FieldsRequirement[] : [];

      const searchTexts: SearchTextRequirement[] = searchSearchRequest.query.texts;
      const searchResults = TableUtility.searchTable(requirements, localRequirements, searchTexts, advSearchCols, dataObjects); // Putting this.dataObjects here enables search on search. Otherwise, use this.dataObjectsBackup
      if (isShowToasterOnSearchNoResult && (!searchResults || searchResults.length === 0)) {
        this.controllerService.invokeInfoToaster('Information', 'No ' + kind + ' records found. Please change search criteria.');
      } else {
        if (isShowToasterOnSearchHasResult) {
          this.controllerService.invokeInfoToaster('Information', 'Found ' + searchResults.length + ' ' + kind + ' records');
        }
      }
      return searchResults;
    } catch (error) {
      this.controllerService.invokeErrorToaster('Error', error.toString());
      return [];
    }
  }

  formatLabels(labelObj) {
    const labels = [];
    if (labelObj != null) {
      Object.keys(labelObj).forEach((key) => {
        labels.push(key + ': ' + labelObj[key]);
      });
    }
    return labels.join(', ');
  }
}

export abstract class CreationForm<I, T extends BaseModel> extends BaseComponent implements OnInit, OnDestroy, AfterViewInit {
  subscriptions: Subscription[] = [];
  newObject: T;

  @Input() isInline: boolean = false;
  @Input() objectData: I;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  isSingleton: boolean = false;

  oldButtons: ToolbarButton[] = [];

  abstract getClassName(): string;
  // Hook to add extra logic during component initialization
  // Defining as abstract to enforce the idea that ngOnInit shouldn't be overriden unless
  // it's really needed.
  abstract postNgInit(): void;
  abstract setToolbar(): void;

  abstract createObject(object: I): Observable<{ body: I | IApiStatus | Error, statusCode: number }>;
  abstract updateObject(newObject: I, oldObject: I): Observable<{ body: I | IApiStatus | Error, statusCode: number }>;
  abstract generateCreateSuccessMsg(object: I): string;
  abstract generateUpdateSuccessMsg(object: I): string;
  abstract isFormValid(): boolean;

  constructor(protected controllerService: ControllerService, protected uiconfigsSerivce: UIConfigsService, protected objConstructor: any = null) {
    super(controllerService, uiconfigsSerivce);
  }


  ngOnInit() {
    this.controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': this.getClassName(), 'state':
        Eventtypes.COMPONENT_INIT
    });
    if (this.objectData != null) {
      this.newObject = new this.objConstructor(this.objectData);
    } else {
      this.newObject = new this.objConstructor();
    }
    this.postNgInit();
    this.setDefaultValidation();
  }

  setDefaultValidation() {
    if (this.isInline) {
      // disable name field
      this.newObject.$formGroup.get(['meta', 'name']).disable();
    }
    this.setCustomValidation();
  }

  // hook for overriding
  setCustomValidation() { }

  ngAfterViewInit() {
    this.replaceToolbar();
    this.postViewInit();
  }

  // hook for overriding
  postViewInit() {}

  replaceToolbar() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this.controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      this.setToolbar();
    }
  }

   /**
   * This API is for table form toolbar buttons style (the [ADD XXX ] button)
   */
  computeButtonClass() {
    if (this.newObject.$formGroup.get('meta.name') && Utility.isEmpty(this.newObject.$formGroup.get('meta.name').value)) {
      return 'global-button-disabled';
    }
    if (this.newObject.$formGroup.get('meta.name').status === 'VALID' && this.isFormValid()) {
      return '';
    } else {
      return 'global-button-disabled';
    }
  }

  /**
   * This API is for inline edit [SAVE] button
   */
  computeInlineButtonClass() {
    if (this.newObject.$formGroup.dirty  && this.isFormValid()  ) {  // don't disable [SAVE] button
      return '';
    } else {  // disable [SAVE] button
      return 'global-button-disabled';
    }
  }

  // Function should be overriden if some values don't reside in the formGroup
  getObjectValues(): I {
    return this.newObject.getFormGroupValues();
  }

  saveObject() {
    // Submit to server
    const policy: I = this.getObjectValues();
    let handler: Observable<{ body: I | IApiStatus | Error, statusCode: number }>;

    // If object is a singleton and the object already exists, we use updateObject(xx).
    if (this.isInline || (this.isSingleton && this.objectData != null)) {
      // the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group
      (<any>policy).meta.name = (<any>this.objectData).meta.name;
      handler = this.updateObject(policy, this.objectData);
    } else {
      handler = this.createObject(policy);
    }

    if (handler == null) {
      return;
    }

    handler.subscribe(
      (response) => {
        // in edit mode or isSingleton, use update message.
        if (this.isInline || this.isSingleton) {
          this.controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, this.generateUpdateSuccessMsg(policy));
          this.onSaveSuccess(false);
        } else {
          this.controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, this.generateCreateSuccessMsg(policy));
          this.onSaveSuccess(true);
        }
        this.cancelObject();
      },
      (error) => {
        if (this.isInline) {
          this.controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
          this.onSaveFailure(false);
        } else {
          this.controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
          this.onSaveFailure(true);
        }
      }
    );
  }

  // Hook for overriding
  onSaveSuccess(isCreate: boolean) { }

  // Hook for overriding
  onSaveFailure(isCreate: boolean) { }

  cancelObject() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit();
  }

  /**
   * Sets the previously saved toolbar buttons
   * They should have been saved in the ngOnInit when we are inline.
   */
  setPreviousToolbar() {
    if (this.oldButtons != null) {
      const currToolbar = this.controllerService.getToolbarData();
      currToolbar.buttons = this.oldButtons;
      this.controllerService.setToolbarData(currToolbar);
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    this.controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': this.getClassName(), 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }

  /**
   * Utility function to check whether input for "timeout" field has value input.
   * @param fieldName
   */
  isTimeoutValid(fieldName: string): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const val: string = control.value;
      if (!val) {
        return null;
      }
      if (!Utility.isTimeoutValid(val)) {
        return {
          [fieldName]: {
            required: false,
            message: 'Invalid time out value. Only h, m, s, ms, us, and ns are allowed'
          }
        };
      }
      return null;
    };
  }

  /**
   * Helper function to add an validator to a form control
   * @param ctrl
   * @param validator
   */
  addFieldValidator(ctrl: AbstractControl, validator: ValidatorFn) {
    if (!ctrl.validator) {
      ctrl.setValidators([validator]);
    } else {
      // check if ctrl.validator
      (Array.isArray(ctrl.validator)) ? ctrl.setValidators([...ctrl.validator, validator]) : ctrl.setValidators([ctrl.validator, validator]);
    }
  }


}
