import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { RadiusComponent } from './radius.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('RadiusComponent', () => {
  let component: RadiusComponent;
  let fixture: ComponentFixture<RadiusComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [RadiusComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        MaterialdesignModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RadiusComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
