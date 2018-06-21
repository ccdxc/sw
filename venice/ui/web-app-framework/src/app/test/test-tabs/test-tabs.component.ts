import { Component, OnInit, ViewEncapsulation, Input, OnChanges } from '@angular/core';
import { FormGroup, FormControl } from '@angular/forms';
import { TabcontentComponent } from '@modules/widgets/penuitabs/tabcontent/tabcontent.component';


@Component({
  selector: 'app-test-sampletab',
  template: `<div class="tab-text">{{text}}</div> <p-button class="edit-button" label="toggleEdit" (onClick)="toggleEdit()"></p-button><p-button label="console log is active tab" (onClick)="activeTab()"></p-button>`,
})
export class SampleTabComponent extends TabcontentComponent implements OnChanges {
  @Input() text: string = 'default text";'
  inEdit: boolean = false;

  toggleEdit() {
    this.inEdit = !this.inEdit;
    this.editMode.emit(this.inEdit);
  }
  activeTab() {
    console.log(this.isActiveTab);
  }

  ngOnChanges(changes) {
    console.log('changes active tab', changes)
  }
}

@Component({
  selector: 'app-test-tabs',
  templateUrl: './test-tabs.component.html',
  styleUrls: ['./test-tabs.component.css'],
  encapsulation: ViewEncapsulation.None
})
export class TestTabsComponent {
  tabCounter1: number = 0;
  tabCounter2: number = 0;
  tabCounter3: number = 0;

  countInc(tab: number) {
    switch (tab) {
      case 1:
        this.tabCounter1++;
        break;
      case 2:
        this.tabCounter2++;
        break;
      case 3:
        this.tabCounter3++;
        break;
    }
  }

}
