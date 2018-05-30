import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';

import { PolicyhealthComponent } from './policyhealth.component';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('PolicyhealthComponent', () => {
  let component: PolicyhealthComponent;
  let fixture: ComponentFixture<PolicyhealthComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [PolicyhealthComponent, DsbdwidgetheaderComponent],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PolicyhealthComponent);
    component = fixture.componentInstance;
    component.id = 'test_id';
    component.background_img = { url: 'test.com' };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
