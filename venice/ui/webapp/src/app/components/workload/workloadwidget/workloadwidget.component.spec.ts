import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import {WidgetsModule} from 'web-app-framework';

import { WorkloadwidgetComponent } from './workloadwidget.component';

describe('WorkloadwidgetComponent', () => {
  let component: WorkloadwidgetComponent;
  let fixture: ComponentFixture<WorkloadwidgetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ WorkloadwidgetComponent ],
      imports: [
        WidgetsModule
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadwidgetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
