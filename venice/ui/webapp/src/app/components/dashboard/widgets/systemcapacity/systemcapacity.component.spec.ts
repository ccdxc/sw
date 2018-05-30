import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';

import { SystemcapacitywidgetComponent } from './systemcapacity.component';
import { DsbdwidgetheaderComponent } from '@components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';


describe('SystemcapacitywidgetComponent', () => {
  let component: SystemcapacitywidgetComponent;
  let fixture: ComponentFixture<SystemcapacitywidgetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SystemcapacitywidgetComponent, DsbdwidgetheaderComponent],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule
      ],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SystemcapacitywidgetComponent);
    component = fixture.componentInstance;
    component.id = 'test_id';
    component.background_img = {
      url: 'test.com'
    };
    component.trendline = {};
    component.menuItems = [];
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
