
static MUSTACHE_RES populate_dot_param_buffer_indv(mustache_parser* prsr, parent_stack* pstack, mustache_param** dot_param_buf, structure* schain, mustache_param*** dot_param_buf_out)
{
    if (schain->type == STRUCTURE_TYPE_VAR)
    {
        structure_vars* v = (structure_vars*)schain;

        uint32_t i;
        for (i = 0; i < v->parameter_count; ++i) {
            structure_parameter_info* pi = v->param_infos + i;

            if (pi->flags & PARAM_FLAG_IS_DOT) {
                structure_foreach* fe_p = (structure_foreach*)pi->untyped_a;


                mustache_param* p = get_foreach_parameter(fe_p);
                uint32_t fe_child_param_count = 0;
                mustache_param* child = mustache_parameter_get_child_list(p, &fe_child_param_count);

                //uintptr_t* offsets = fe_p->cur_dot_offset + fe_child_param_count;
                //fe_p->cur_dot_offset+=fe_child_param_count
                uint32_t initial_dot_offset = fe_p->cur_dot_offset;
                uint32_t j;
                for (j = 0; j < fe_child_param_count; j++)
                {
                    fe_p->param_index = j;
                    //if (j == fe_p->param_index) {
                    parent_stack_push(prsr, pstack, child, (structure*)NULL, NULL);

                    mustache_param* param = structure_var_evaluate_dot_parameter(prsr, pstack, fe_p, j, pi);
                    if (!param) {
                        char err_msg[256];
                        snprintf(err_msg, sizeof(err_msg), "error: could not evaluate dot parameter.");
                        prsr->err_callback(prsr, err_msg, pi->first, pi->end);
                        return MUSTACHE_ERR;
                    }

                    /*
                    #ifndef  NDEBUG
                        printf("\n dot parameter count: %d\n", fe_p->DBG_dot_param_count);
                        mustache_print_parameter_list(param);
                        printf("\n");
                    #endif // ! NDEBUG
                    */

                    fe_p->dot_params[fe_p->cur_dot_offset++] = param;
                    mustache_print_node(param,0);

                    parent_stack_pop(pstack);
                    child = child->pNext;
                    //}
                }
                if (!(pi->flags & PARAM_FLAG_UNIQUE)) {
                    pi->flags |= PARAM_FLAG_UNIQUE;
                    v->parameters[i] = (void*)initial_dot_offset; // set parameter to index within the parent foreach structure dot parameter list
                }
                // the actual parameter is acessed by: fe.dot_params[initial_dot_offset + fe.current_index]
            }
        }
    }
    else if (schain->type == STRUCTURE_TYPE_FOREACH) {
        structure_foreach* fe = (structure_foreach*)schain;
        fe->cur_dot_offset = 0;

        uint32_t dot_param_count = (uint32_t)fe->dot_params;
        fe->dot_params = dot_param_buf;
        dot_param_buf += dot_param_count * 2;


        if (fe->flags & PARAM_FLAG_IS_DOT) {
            structure_foreach* fe_parent = fe->dotfe_parent;

            // get parent information
            mustache_param* fe_p_param = get_foreach_parameter(fe_parent);
            uint32_t fe_p_child_param_count = 0;
            mustache_param* fe_p_children_parameters = mustache_parameter_get_child_list(fe_p_param, &fe_p_child_param_count);



            // evaluate dot parameters
            fe->param = (void*)fe_parent->cur_dot_offset;
            uint32_t j = 0;
            for (j = 0; j < fe_p_child_param_count; ++j) {
                parent_stack_push(prsr, pstack, fe_p_children_parameters, fe_parent, NULL);
                fe_parent->param_index = j;

                mustache_param* p = structure_foreach_evaluate_dot_parameter(prsr, pstack, fe_parent, fe_parent->param_index, fe);

                fe_parent->dot_params[fe_parent->cur_dot_offset++] = p;
                mustache_print_node(p, 0);


                // populate for children
                mustache_param* fe_param_list = get_foreach_parameter(fe);
                uint32_t fe_child_param_count;
                mustache_param* child = mustache_parameter_get_child_list(fe_param_list, &fe_child_param_count);
                MUSTACHE_RES r = populate_dot_param_buffer(prsr, pstack, dot_param_buf, fe->children);

                parent_stack_pop(pstack);
                fe_p_children_parameters = fe_p_children_parameters->pNext;
            }


        }
        else 
        {
            uint32_t fe_child_param_count;
            mustache_param* child = mustache_parameter_get_child_list(get_foreach_parameter(fe), &fe_child_param_count);
            MUSTACHE_RES r = populate_dot_param_buffer(prsr, pstack, dot_param_buf, fe->children);
        }



        /*
        uint32_t child_count;
        mustache_param* fe_param = get_foreach_parameter(fe);
        mustache_param* param_child = mustache_parameter_get_child_list(fe_param, &child_count);

        if (child_count) 
        {
            uint32_t dot_param_count = (uint32_t)fe->dot_params;
            fe->dot_params = dot_param_buf;

            fe->dotoffsets_of_index = prsr->alloc(prsr, sizeof(uint32_t) * child_count);
            if (!fe->dotoffsets_of_index) {
                return MUSTACHE_ERR_ALLOC;
            }


            dot_param_buf += dot_param_count * 2;


            //for (fe->param_index = 0; fe->param_index < child_count; ++fe->param_index)
            //{

                parent_stack_push(prsr, pstack, param_child, fe, NULL);
                MUSTACHE_RES r = populate_dot_param_buffer(prsr, pstack, dot_param_buf, fe->children);
                if (r) {
                    return r;
                }

                parent_stack_pop(pstack);

                param_child = param_child->pNext;

                // if (fe->param_index != child_count - 1) {
                //    fe->dotoffsets_of_index[fe->param_index + 1] = fe->cur_dot_offset;
                //}
            //}
        }
        */
    }

    *dot_param_buf_out = dot_param_buf;

    return MUSTACHE_SUCCESS;
}