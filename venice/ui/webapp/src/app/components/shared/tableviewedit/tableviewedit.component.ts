import { AfterViewInit, ChangeDetectorRef, Component, EventEmitter, Input, IterableDiffer, OnChanges, OnDestroy, OnInit, Output, SimpleChanges, TemplateRef, ViewChild, ViewEncapsulation, Renderer2 } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { IApiStatus } from '@sdk/v1/models/generated/search';
import { Table } from 'primeng/table';
import { Observable, Subscription } from 'rxjs';
import { TabcontentInterface } from 'web-app-framework';
import { LazyrenderComponent } from '../lazyrender/lazyrender.component';
import { LazyLoadEvent } from 'primeng/primeng';
import { TableCol, RowClickEvent } from '.';
import { TableMenuItem } from '../tableheader/tableheader.component';
import { TableUtility } from './tableutility';


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

  constructor(protected renderer: Renderer2) {
  }

  ngOnInit() {
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

  updateWidthPercentages() {
    const tableWidth = $(this.headerRow.nativeElement).innerWidth();
    if (tableWidth !== 0) {
      this.actionWidth = this.actionWidthPx / tableWidth; // now converted to percentage
      let sum = 0;
      if (tableWidth !== 0) {
        const children = this.headerRow.nativeElement.children;
        for (let i = 0; i < children.length - 1; i++) {
          const newWidth = ($(children[i]).outerWidth() * 100) / tableWidth;
          this.cols[i].width = newWidth;
          sum += newWidth;
        }
        this.cols[children.length - 1].width = 100 - sum;
      }
    }
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
    this.leftCellWidth = $(children[this.headerIndex]).outerWidth();
    this.rightCellWidth = $(children[this.headerIndex + 1]).outerWidth();
  }

  getNumber(num: number | string ): number {
    return typeof num === 'number' ? num : null;
  }

  setNewWidths(displacement) {
    const tableWidth = $(this.headerRowHandler).width();

    const leftMinWidth = (this.cols[this.headerIndex].minColumnWidth) ? this.cols[this.headerIndex].minColumnWidth : TablevieweditHTMLComponent.MIN_COLUMN_WIDTH;
    let rightMinWidth = (this.cols[this.headerIndex].minColumnWidth) ? this.cols[this.headerIndex + 1].minColumnWidth : TablevieweditHTMLComponent.MIN_COLUMN_WIDTH;

    // If last two columns are being resized, we need to worry about actionWidth
    if (this.headerIndex === this.cols.length - 2) {
      rightMinWidth += this.actionWidthPx;
    }

    if ( (displacement < 0 && this.leftCellWidth + displacement > leftMinWidth) || (displacement > 0 && this.rightCellWidth - displacement > rightMinWidth) ) {
      let sum = 0;
      // Incase width is defined using strings, we convert it to percentages first.
      if (typeof this.cols[this.headerIndex].width === 'string' || typeof this.cols[this.headerIndex + 1].width === 'string') {
        this.updateWidthPercentages();
      }
      sum = this.getNumber(this.cols[this.headerIndex].width) + this.getNumber(this.cols[this.headerIndex + 1].width);
      this.cols[this.headerIndex].width = (this.leftCellWidth + displacement) * 100 / tableWidth;
      this.cols[this.headerIndex + 1].width = sum - this.getNumber(this.cols[this.headerIndex].width);
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
}

export abstract class TableviewAbstract<I, T extends I> implements OnInit, OnDestroy, OnChanges, TabcontentInterface {
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
      text: 'Export',
      onClick: () => {
        this.exportTableData();
      }
    }
  ];


  abstract dataObjects: ReadonlyArray<T> = [];
  // Whether or not this component is a tab
  abstract isTabComponent: boolean;
  abstract disableTableWhenRowExpanded: boolean;

  abstract exportFilename: string;
  abstract cols: TableCol[];

  abstract getClassName(): string;
  abstract setDefaultToolbar(): void;

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef) {
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

  // Hook to add logic to the initialization for classes extending this component
  postNgInit() { }

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

  exportTableData() {
    TableUtility.exportTable(this.cols, this.dataObjects, this.exportFilename);
    this.controllerService.invokeInfoToaster('File Exported', this.exportFilename + '.csv');
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

}

export abstract class TablevieweditAbstract<I, T extends I> extends TableviewAbstract<I, T> {
  creatingMode: boolean = false;

  abstract deleteRecord(object: T): Observable<{ body: I | IApiStatus | Error, statusCode: number }>;
  abstract generateDeleteConfirmMsg(object: T): string;
  abstract generateDeleteSuccessMsg(object: T): string;

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
  postDeleteRecord() {}

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
}
