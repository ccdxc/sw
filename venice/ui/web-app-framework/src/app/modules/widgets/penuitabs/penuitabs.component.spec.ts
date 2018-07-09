import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PenuitabsComponent } from './penuitabs.component';
import { Component, Input, OnChanges, ViewChild, SimpleChanges } from '@angular/core';
import { PenuitabComponent } from '@modules/widgets/penuitabs/penuitab/penuitab.component';
import { TabcontentComponent } from '@modules/widgets/penuitabs/tabcontent/tabcontent.component';
import { By } from '@angular/platform-browser';
import { MaterialModule } from '../../../material.module';
import { PrimengModule } from '../../../primeng.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

@Component({
  selector: 'test',
  template: `<app-penuitabs>
              <app-penuitab title="TAB 1" [count]="tabCounter1">
                <ng-template>
                  <app-test-sampletab #tabContent text="tab 1">
                  </app-test-sampletab>
                </ng-template>
              </app-penuitab>
              <app-penuitab title="TAB 2" [count]="tabCounter2">
                <ng-template>
                  <app-test-sampletab #tabContent text="tab 2">
                  </app-test-sampletab>
                </ng-template>
              </app-penuitab>
              <app-penuitab title="TAB 3" [count]="tabCounter3">
                <ng-template>
                  <app-test-sampletab #tabContent text="tab 3-1">
                  </app-test-sampletab>
                  <app-test-sampletab #tabContent text="tab 3-2">
                  </app-test-sampletab>
                </ng-template>
              </app-penuitab>
            </app-penuitabs>`,
})
class TestWrapperComponent {
  @ViewChild(PenuitabsComponent) tabComponent: PenuitabsComponent;
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

@Component({
  selector: 'app-test-sampletab',
  template: `<div class="tab-text">{{text}}</div> 
            <p-button class="edit-button" label="toggleEdit" (onClick)="toggleEdit()">`,
})
export class SampleTabComponent extends TabcontentComponent implements OnChanges {
  @Input() text: string = 'default text";'
  inEdit: boolean = false;

  toggleEdit() {
    this.inEdit = !this.inEdit;
    this.editMode.emit(this.inEdit);
  }

  ngOnChanges(changes) {
  }
}

describe('PenuitabsComponent', () => {
  let component: PenuitabsComponent;
  let fixture: ComponentFixture<TestWrapperComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestWrapperComponent,
        PenuitabsComponent,
        PenuitabComponent,
        SampleTabComponent
      ],
      imports: [MaterialModule, PrimengModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestWrapperComponent);
    component = fixture.componentInstance.tabComponent;
  });

  /**
   * Test that all the tab contents subscriptions
   * for edit mode and for isActiveTab are setup
   */
  it('should subscribe to all penuitab', () => {
    fixture.detectChanges();
    // waiting for the templates to render
    fixture.whenRenderingDone().then(() => {
      // after templates render, need a second
      // detect changes to trigger the contentChildren 
      // of penuitab to be picked up (#tabContent)
      fixture.detectChanges();

      component.tabItems.forEach((tab, i) => {
        if (i === 2) {
          expect(tab.content.length).toBe(2);
        } else {
          expect(tab.content.length).toBe(1);
        }

        tab.content.forEach((content) => {
          // Check that penuitabs has subscribed
          expect(content.editMode.observers.length).toBe(1);
          // The first tab should be selected by default
          expect(content.isActiveTab).toBe(i === 0);
        })
      })
      // Test active tabs and the simple changes are done correctly
      // Check that the ngOnChanges is triggered
      const spy0 = spyOn(component.tabItems.toArray()[0].content.first, 'ngOnChanges');
      const spy1 = spyOn(component.tabItems.toArray()[1].content.first, 'ngOnChanges');
      const spy2 = spyOn(component.tabItems.toArray()[2].content.first, 'ngOnChanges');
      component.changeSelectedTab(1);
      fixture.detectChanges();
      component.tabItems.forEach((tab, i) => {
        tab.content.forEach((content) => {
          expect(content.isActiveTab).toBe(i === 1);
        })
      });

      expect(spy0).toHaveBeenCalled();
      expect(spy0).toHaveBeenCalledTimes(1);
      let args = spy0.calls.argsFor(0)[0];
      expect(args.isActiveTab).toBeDefined();
      expect(args.isActiveTab.previousValue).toBeTruthy();
      expect(args.isActiveTab.currentValue).toBeFalsy();
      expect(args.isActiveTab.firstChange).toBeFalsy();
      expect(args.isActiveTab.isFirstChange()).toBeFalsy();

      expect(spy1).toHaveBeenCalled();
      args = spy1.calls.argsFor(0)[0];
      expect(args.isActiveTab.previousValue).toBeFalsy();
      expect(args.isActiveTab.currentValue).toBeTruthy();
      expect(args.isActiveTab.firstChange).toBeFalsy();
      expect(args.isActiveTab.isFirstChange()).toBeFalsy();

      // the last tab's value never changed, so there should be no ngOnChanges call
      expect(spy2).toHaveBeenCalledTimes(0);

    });
  });


  it('should display tab headers and content corretly', () => {
    // Check can't change tab while in edit mode, but can change after
    component.startingIndex = 2;
    const spy0 = spyOn(component.selectedIndexChange, 'emit');
    fixture.detectChanges();

    // waiting for the templates to render
    fixture.whenRenderingDone().then(() => {
      // after templates render, need a second
      // detect changes to trigger the contentChildren 
      // of penuitab to be picked up (#tabContent)
      fixture.detectChanges();
      // There should be 3 tabs, last tab is active
      expect(spy0).toHaveBeenCalledTimes(1);
      expect(spy0).toHaveBeenCalledWith(2);
      spy0.calls.reset();
      const tabs = fixture.debugElement.queryAll(By.css('.mat-tab-label'));
      expect(tabs.length).toBe(3);
      let activeTab = fixture.debugElement.queryAll(By.css('.mat-tab-label-active'));
      expect(activeTab.length).toBe(1);
      let label = activeTab[0].children[0].children[0];
      expect(label.children[0].nativeElement.textContent).toBe('TAB 3');
      expect(label.children[2].nativeElement.textContent).toBe('0');

      // Checking the displayed content of the tab
      let tabBodies = fixture.debugElement.queryAll(By.css('.mat-tab-body-active'));
      expect(tabBodies.length).toBe(1);
      expect(tabBodies[0].queryAll(By.css('.tab-text'))[0].nativeElement.textContent).toBe('tab 3-1');
      expect(tabBodies[0].queryAll(By.css('.tab-text'))[1].nativeElement.textContent).toBe('tab 3-2');

      // Check that the badge and text updates
      component.tabItems.last.title = "NEW TAB 3";
      component.tabItems.last.count = 4;
      fixture.detectChanges();

      expect(label.children[0].nativeElement.textContent).toBe('NEW TAB 3');
      expect(label.children[2].nativeElement.textContent).toBe('4');

      // // Check entering edit mode
      const editButton = tabBodies[0].query(By.css('.edit-button'));
      editButton.children[0].nativeElement.click();
      fixture.detectChanges();
      expect(component.editMode).toBeTruthy();
      let disabledLabels = fixture.debugElement.queryAll(By.css('.mat-tab-disabled'));
      expect(disabledLabels.length).toBe(2);
      expect(disabledLabels[0].nativeElement.children[0].children[0].children[0].textContent).toBe('TAB 1');
      expect(disabledLabels[1].nativeElement.children[0].children[0].children[0].textContent).toBe('TAB 2');

      // check cant change current tab
      expect(component.changeSelectedTab(1)).toBe(false);

      // Exit edit mode
      editButton.children[0].nativeElement.click();
      fixture.detectChanges();
      disabledLabels = fixture.debugElement.queryAll(By.css('.mat-tab-disabled'));
      expect(disabledLabels.length).toBe(0);

      // should now be able to switch tabs
      expect(component.changeSelectedTab(1)).toBe(true);
      fixture.detectChanges();
      expect(spy0).toHaveBeenCalledTimes(1);
      expect(spy0).toHaveBeenCalledWith(1);
      activeTab = fixture.debugElement.queryAll(By.css('.mat-tab-label-active'));
      label = activeTab[0].children[0].children[0]
      expect(label.children[0].nativeElement.textContent).toBe('TAB 2');
      expect(label.children[2].nativeElement.textContent).toBe('0');

    });
  })

  it('should reject if no penuitab is given', () => {
    fixture.detectChanges();
    fixture.whenStable().then(() => {
      component.tabItems = undefined;
      expect(() => component.ngAfterContentInit())
        .toThrowError('No Penuitab components were given');
    })
  })
});
