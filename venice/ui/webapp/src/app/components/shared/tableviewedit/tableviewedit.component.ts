import { AfterViewInit, ChangeDetectorRef, Component, EventEmitter, Input, IterableDiffer, OnChanges, OnDestroy, OnInit, Output, SimpleChanges, TemplateRef, ViewChild, ViewEncapsulation } from '@angular/core';
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
  @ViewChild('primengTable') table: Table;
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

  actionWidth: number = 5;

  displayedItemCount: number = null;

  ngOnInit() {
    // All the col widths need to add up to 100
    // Action buttons take up 5% by default
    let total = 0;
    this.cols.forEach((c) => {
      total += c.width;
    });
    if (total !== 100) {
      console.error('Table columns don\'t add up to 100, added up to ' + total);
    }
  }

  dataUpdated() {
    this.displayedItemCount = this.lazyRenderWrapper.getCurrentDataLength();
  }


  ngAfterViewInit() {
    // Pushing into the next change detection cycle
    setTimeout(() => {
      this.displayedItemCount = this.lazyRenderWrapper.getCurrentDataLength();
    }, 0);
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
            const errorMsg = error.body != null ? error.body.message : '';
            this.controllerService.invokeErrorToaster(Utility.DELETE_FAILED_SUMMARY, errorMsg);
          }
        );
        this.subscriptions.push(sub);
      }
    });
  }
}
