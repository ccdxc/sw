import { AfterContentInit, Component, ContentChildren, OnDestroy, OnInit, QueryList, SimpleChanges, ViewEncapsulation, ChangeDetectorRef, Input, Output, EventEmitter } from '@angular/core';
import { PenuitabComponent } from './penuitab/penuitab.component';

/**
 * Tab group component that uses Material design tabs
 * Since disabling tab switching while the user is 
 * editing data is a common pattern, we put all this logic into
 * a custom tab component.
 * 
 * This component uses penuitab component as a way to 
 * detect tabs and their label metadata (title/count)
 * The penuitab then looks for an ng-template in its immediate children
 * which it will pass to penuitabs to plug in into the tab's body.
 * It also looks for any components with a #tabContent selector and will then
 * try to subscribe to an editMode observer on the component as well as pass in
 * whether the component is on an active tab. Components that use the #tabContent selector should
 * extend from tabcontent component, which has these properties already.
 */
@Component({
  selector: 'app-penuitabs',
  templateUrl: './penuitabs.component.html',
  styleUrls: ['./penuitabs.component.css'],
  encapsulation: ViewEncapsulation.None
})
export class PenuitabsComponent implements OnInit, OnDestroy, AfterContentInit {
  @ContentChildren(PenuitabComponent) tabItems: QueryList<any>;

  private _selectedIndex: number = 0;
  subscriptions = [];
  editMode: boolean = false;

  /** The index of the active tab. */
  get selectedIndex(): number | null { return this._selectedIndex; }
  set selectedIndex(value: number) {
    if (value == null) {
      value = 0;
    }
    this._selectedIndex = value;
    this.selectedIndexChange.emit(this._selectedIndex);
    this.updateIsActiveTab();
  }

  @Input() startingIndex: number;
  @Output() selectedIndexChange: EventEmitter<number> = new EventEmitter<number>();

  constructor() { }

  ngOnInit() {
    if (typeof (this.startingIndex) === 'string') {
      throw new Error("staringIndex must be a number");
    }
    this.selectedIndex = this.startingIndex;
  }

  /**
   * Will change the selected tab, unless we are in editMode
   * @param value index of the tab to change to
   */
  changeSelectedTab(value: number): boolean {
    if (value == null || this.editMode) {
      return false;
    }
    this.selectedIndex = value;
    return true;
  }

  ngAfterContentInit() {
    if (this.tabItems.length == 0) {
      throw new Error('No Penuitab components were given');
    }
    this.tabItems.forEach((tabContainer) => {
      // waiting for the penuitab to have its contents ready
      const tabSubscription = tabContainer.contentChanged.subscribe(() => {
        tabContainer.content.forEach((content) => {
          // If content has the editMode property, we will subscribe
          // to it.
          if (content.editMode) {
            const subscription = content.editMode.subscribe((val: boolean) => {
              this.toggleTabsActive(val);
            })
            this.subscriptions.push(subscription);
          }
        })
        this.updateIsActiveTab();
      })
      this.subscriptions.push(tabSubscription);
    })
  }

  /**
   * Updates the isActiveTab attribute of all the components set
   * to be tba contents
   */
  updateIsActiveTab() {
    if (this.tabItems == null) {
      return;
    }
    this.tabItems.forEach((tabContainer, index) => {
      tabContainer.content.forEach((content) => {
        // Since we are changing the binding value ourselves,
        // Angular doesn't know that the value has changed, so
        // we trigger the change event manually

        // check if it has this property
        if (!content.hasOwnProperty('isActiveTab')) {
          return;
        }
        const prevVal = content.isActiveTab;
        const newVal = this._selectedIndex === index;
        if (prevVal === newVal) {
          return;
        }
        content.isActiveTab = newVal;
        // We always put firstChange as false
        // It is possible to add functionality to track
        // whether this is the first change, but there is no
        // current use case for this.
        const changes: SimpleChanges = {
          isActiveTab: {
            previousValue: prevVal,
            currentValue: newVal,
            firstChange: false,
            isFirstChange: () => { return false; }
          }
        }
        if (content.ngOnChanges != null) {
          content.ngOnChanges(changes);
        }
      });
    });
  }

  toggleTabsActive(event) {
    if (this.editMode != event) {
      this.editMode = event;
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
  }
}
