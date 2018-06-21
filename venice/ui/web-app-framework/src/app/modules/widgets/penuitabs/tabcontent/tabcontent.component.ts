import { Component, EventEmitter, Input, Output, OnChanges, ChangeDetectorRef, SimpleChanges } from '@angular/core';

/**
 * Basic tab content component. Any component that is set to be the
 * content of penuitab should inherit this.
 * 
 * Since there is a common pattern of a tab needing
 * to disable all other tabs while user is editing or creating
 * a form, we put all this logic in a base class.
 * 
 * This will automatically bind to the isActiveTab 
 * passed from penuitabs, and penuitabs will automatically
 * listen to this components editMode. A component just needs to emit
 * an editMode event while it is the active tab to disable other tabs
 * 
 */
@Component({
  selector: 'app-tabcontent',
  templateUrl: './tabcontent.component.html',
  styleUrls: ['./tabcontent.component.css']
})
export class TabcontentComponent implements OnChanges {
  // Will be set by penuitabs
  @Input() isActiveTab: boolean = false;
  // will listen for by penuitabs and will disable the rest of the tabs
  @Output() editMode: EventEmitter<any> = new EventEmitter();

  constructor() {
  }

  /**
   * Hook so that penuitabs can call when it updates isActiveTab
   */
  ngOnChanges(changes: SimpleChanges) {
  }
}
