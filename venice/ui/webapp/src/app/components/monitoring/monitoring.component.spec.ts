import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MonitoringComponent } from './monitoring.component';
import { RouterTestingModule } from '@angular/router/testing';

describe('MonitoringComponent', () => {
  let component: MonitoringComponent;
  let fixture: ComponentFixture<MonitoringComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [MonitoringComponent],
      imports: [
        RouterTestingModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MonitoringComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
