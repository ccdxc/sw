import { Component, OnInit, Input, ContentChildren, QueryList, TemplateRef, Output, EventEmitter, ViewChildren, AfterContentInit, ContentChild } from '@angular/core';

@Component({
  selector: 'app-penuitab',
  templateUrl: './penuitab.component.html',
  styleUrls: ['./penuitab.component.css']
})
export class PenuitabComponent implements AfterContentInit {
  // Used for the tab heading
  @Input() title: string;
  @Input() count: number;

  // Used by penuitabs to pick up the content and display it as the tab content
  @ContentChild(TemplateRef) template: TemplateRef<any>;

  // Used by penuitabs to access the component that is contained
  // Will subscribe to the editmode and pass in the isActiveTab
  // to the component
  @ContentChildren("tabContent") content: QueryList<any>;
  @Output() contentChanged: EventEmitter<boolean> = new EventEmitter();

  constructor() { }

  ngAfterContentInit() {
    if (this.template == null) {
      throw new Error('Tab Content needs to be surrounded by an ng template');
    }
    // penuitabs is waiting for this tab's content children to be ready
    // before setting up the listeners
    this.content.changes.subscribe(() => {
      this.contentChanged.emit(true);
    })
  }

}
