import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PenuitabComponent } from './penuitab.component';
import { ViewChild, Component } from '@angular/core';

/**
 * Wrapper component to test the tab functionality
 * The template must be rendered for the content children
 * to pick up nested values (#tabContent).
 */
@Component({
  selector: 'test',
  template: `<app-penuitab title="TAB 1" [count]="tabCounter1">
                <ng-template #test>
                  <div #tabContent>
                  </div>
                </ng-template>
              </app-penuitab>
              <ng-container *ngTemplateOutlet="test">`,
})
class TestWrapperComponent {
  @ViewChild(PenuitabComponent) tabComponent: PenuitabComponent;
}

describe('PenuitabComponent', () => {
  let component: PenuitabComponent;
  let fixture: ComponentFixture<TestWrapperComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestWrapperComponent, PenuitabComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestWrapperComponent);
    component = fixture.componentInstance.tabComponent;
    fixture.detectChanges();
  });

  it('should find its content children', () => {
    expect(component.content.length).toBeGreaterThan(0);
    expect(component.template).toBeDefined();;
  });

});
