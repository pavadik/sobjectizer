/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Agent cooperation definition.
*/

#if !defined( _SO_5__RT__AGENT_COOP_HPP_ )
#define _SO_5__RT__AGENT_COOP_HPP_

#include <vector>
#include <memory>
#include <functional>

#include <so_5/h/declspec.hpp>
#include <so_5/h/exception.hpp>
#include <so_5/h/types.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/adhoc_agent_wrapper.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class agent_core_t;
class agent_coop_private_iface_t;
class so_environment_impl_t;

} /* namespace impl */

class environment_t;
class agent_coop_t;

namespace dereg_reason
{

/*!
 * \name Cooperation deregistration reasons.
 * \{
 */
//! Normal deregistration.
const int normal = 0;

//! Deregistration because SObjectizer Environment shutdown.
const int shutdown = 1;

//! Deregistration because parent cooperation deregistration.
const int parent_deregistration = 2;

//! Deregistration because of unhandled exception.
const int unhandled_exception = 3;

//! Deregistration because of unknown error.
const int unknown_error = 4;

//! Reason is not properly defined.
const int undefined = -1;

//! A starting point for user-defined reasons.
const int user_defined_reason = 0x1000;
/*!
 * \}
 */

} /* namespace dereg_reason */

//
// coop_dereg_reason_t
//
/*!
 * \since v.5.2.3
 */
class coop_dereg_reason_t
{
	public :
		inline coop_dereg_reason_t()
			:	m_reason( dereg_reason::undefined )
		{}

		inline explicit coop_dereg_reason_t( int reason )
			:	m_reason( reason )
		{}

		inline int
		reason() const
		{
			return m_reason;
		}

		inline void
		swap( coop_dereg_reason_t & o )
		{
			std::swap( m_reason, o.m_reason );
		}

	private :
		int m_reason;
};

//
// coop_reg_notificator_t
//
/*!
 * \since v.5.2.3
 * \brief Type of cooperation registration notificator.
 *
 * Cooperation notificator should be a function with the following
 * prototype:
\code
void
notificator(
	// SObjectizer Environment for cooperation.
	so_5::rt::environment_t & env,
	// Name of cooperation.
	const std::string & coop_name );
\endcode
 */
typedef std::function<
				void(environment_t &, const std::string &) >
		coop_reg_notificator_t;

//
// coop_reg_notificators_container_t
//
/*!
 * \since v.5.2.3
 * \brief Container for cooperation registration notificators.
 */
class SO_5_TYPE coop_reg_notificators_container_t
	:	public atomic_refcounted_t
{
	public :
		coop_reg_notificators_container_t();
		~coop_reg_notificators_container_t();

		//! Add a notificator.
		void
		add(
			const coop_reg_notificator_t & notificator );

		//! Call all notificators.
		/*!
		 * \note All exceptions are suppressed.
		 */
		void
		call_all(
			environment_t & env,
			const std::string & coop_name ) const;

	private :
		std::vector< coop_reg_notificator_t > m_notificators;
};

//
// coop_reg_notificators_container_ref_t
//
/*!
 * \since v.5.2.3
 * \brief Typedef for smart pointer to notificators_container.
 */
typedef smart_atomic_reference_t< coop_reg_notificators_container_t >
	coop_reg_notificators_container_ref_t;

//
// coop_dereg_notificator_t
//
/*!
 * \since v.5.2.3
 * \brief Type of cooperation deregistration notificator.
 *
 * Cooperation notificator should be a function with the following
 * prototype:
\code
void
notificator(
	// SObjectizer Environment for cooperation.
	so_5::rt::environment_t & env,
	// Name of cooperation.
	const std::string & coop_name,
	// Reason of deregistration.
	const so_5::rt::coop_dereg_reason_t & reason );
\endcode
 */
typedef std::function<
				void(
						environment_t &,
						const std::string &,
						const coop_dereg_reason_t &) >
		coop_dereg_notificator_t;

//
// coop_dereg_notificators_container_t
//
/*!
 * \since v.5.2.3
 * \brief Container for cooperation deregistration notificators.
 */
class SO_5_TYPE coop_dereg_notificators_container_t
	:	public atomic_refcounted_t
{
	public :
		coop_dereg_notificators_container_t();
		~coop_dereg_notificators_container_t();

		//! Add a notificator.
		void
		add(
			const coop_dereg_notificator_t & notificator );

		//! Call all notificators.
		/*!
		 * \note All exceptions are suppressed.
		 */
		void
		call_all(
			environment_t & env,
			const std::string & coop_name,
			const coop_dereg_reason_t & reason ) const;

	private :
		std::vector< coop_dereg_notificator_t > m_notificators;
};

//
// coop_dereg_notificators_container_ref_t
//
/*!
 * \since v.5.2.3
 * \brief Typedef for smart pointer to notificators_container.
 */
typedef smart_atomic_reference_t< coop_dereg_notificators_container_t >
	coop_dereg_notificators_container_ref_t;

//! Agent cooperation.
/*!
 * The main purpose of the cooperation is the introducing of several agents into
 * SObjectizer as a single unit. A cooperation should be registered.
 *
 * For the cooperation to be successfuly registered all of its agents must 
 * successfuly pass registration steps (so-define, bind to the dispatcher). 
 * If at least one agent out of this cooperation fails to pass any of 
 * mentioned steps, the cooperation will not be registered and 
 * all of agents will run procedures opposite to registration 
 * steps (unbind from the dispatcher, so-undefine) which had been successfuly 
 * taken for the particulary agent in the reverse order.
 *
 * Agents are added to the cooperation by the add_agent() method.
 *
 * After addition to the cooperation the cooperation takes care about
 * the agent lifetime.
 */
class SO_5_TYPE agent_coop_t
{
	private :
		friend class agent_t;
		friend class impl::agent_core_t;
		friend class impl::agent_coop_private_iface_t;

	protected :
		virtual ~agent_coop_t();

	public:
		/*!
		 * \since v.5.2.3
		 * \brief Deleter for agent_coop.
		 */
		static void
		destroy( agent_coop_t * coop );

		//! Constructor.
		agent_coop_t(
			//! Cooperation name.
			const nonempty_name_t & name,
			//! Default dispatcher binding.
			disp_binder_unique_ptr_t coop_disp_binder,
			//! SObjectizer Environment.
			environment_t & env );

		//! Get cooperation name.
		const std::string &
		query_coop_name() const;

		//! Add agent to cooperation.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		template< class AGENT >
		inline AGENT *
		add_agent(
			//! Agent.
			std::unique_ptr< AGENT > agent )
		{
			AGENT * p = agent.get();
			this->do_add_agent( agent_ref_t( agent.release() ) );

			return p;
		}

		//! Add agent to cooperation via raw pointer.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		template< class AGENT >
		inline AGENT *
		add_agent(
			//! Agent.
			AGENT * agent )
		{
			this->do_add_agent( agent_ref_t( agent ) );

			return agent;
		}

		//! Add agent to the cooperation with the dispatcher binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		template< class AGENT >
		inline AGENT *
		add_agent(
			//! Agent.
			std::unique_ptr< AGENT > agent,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder )
		{
			AGENT * p = agent.get();

			this->do_add_agent(
				agent_ref_t( agent.release() ),
				std::move(disp_binder) );

			return p;
		}

		//! Add agent to the cooperation via raw pointer and with the dispatcher
		//! binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		template< class AGENT >
		inline AGENT *
		add_agent(
			//! Agent.
			AGENT * agent,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder )
		{
			this->do_add_agent(
				agent_ref_t( agent ),
				std::move(disp_binder) );

			return agent;
		}

		//! Internal SObjectizer method.
		/*!
		 * \since v.5.2.3
		 *
		 * Informs cooperation that it is used by yet another entity.
		 */
		static inline void
		increment_usage_count( agent_coop_t & coop )
		{
			coop.increment_usage_count();
		}

		//! Internal SObjectizer method.
		/*!
		 * Informs cooperation about full finishing of agent's or
		 * child cooperation work.
		 */
		static inline void
		decrement_usage_count( agent_coop_t & coop )
		{
			coop.decrement_usage_count();
		}

		//! Internal SObjectizer method.
		/*!
		 * Initiate a final deregistration stage.
		 */
		static inline void
		call_final_deregister_coop( agent_coop_t * coop )
		{
			coop->final_deregister_coop();
		}

		/*!
		 * \name Methods for working with name of parent cooperation.
		 * \{
		 */
		/*!
		 * \since v.5.2.3
		 * \brief Does cooperation have parent cooperation?
		 */
		bool
		has_parent_coop() const;

		/*!
		 * \since v.5.2.3
		 * \brief Set name of the parent cooperation.
		 */
		void
		set_parent_coop_name(
			const nonempty_name_t & name );

		/*!
		 * \since v.5.2.3
		 * \brief Get name of the parent cooperation.
		 *
		 * \throw exception_t if the parent cooperation name is not set.
		 */
		const std::string &
		parent_coop_name() const;
		/*!
		 * \}
		 */

		/*!
		 * \name Method for working with notificators.
		 * \{
		 */
		/*!
		 * \since v.5.2.3
		 * \brief Add notificator about cooperation registration event.
		 */
		void
		add_reg_notificator(
			const coop_reg_notificator_t & notificator );

		/*!
		 * \since v.5.2.3
		 * \brief Add notificator about cooperation deregistration event.
		 */
		void
		add_dereg_notificator(
			const coop_dereg_notificator_t & notificator );
		/*!
		 * \}
		 */

		/*!
		 * \name Method for working with user resources.
		 * \{
		 */
		/*!
		 * \since v.5.2.3
		 * \brief Take a user resouce under cooperation control.
		 */
		template< class T >
		T *
		take_under_control( std::unique_ptr< T > resource )
		{
			T * ret_value = resource.get();

			resource_deleter_t d = [ret_value]() { delete ret_value; };
			m_resource_deleters.emplace_back( std::move(d) );

			resource.release();

			return ret_value;
		}

		/*!
		 * \since v.5.2.3
		 * \brief Take a user resouce under cooperation control.
		 */
		template< class T >
		T *
		take_under_control( T * resource )
		{
			return take_under_control( std::unique_ptr< T >( resource ) );
		}
		/*!
		 * \}
		 */

		/*!
		 * \name Exception reaction methods.
		 * \{
		 */
		/*!
		 * \since v.5.3.0
		 * \brief Set exception reaction for that cooperation.
		 *
		 * This value will be used by agents and children cooperation if
		 * they use inherit_exception_reaction value.
		 */
		void
		set_exception_reaction(
			exception_reaction_t value );

		/*!
		 * \since v.5.3.0
		 * \brief Get the current exception rection flag for that cooperation.
		 *
		 * It uses the following logic:
		 * - if own's exception_reaction flag value differs from
		 *   inherit_exception_reaction value than own's exception_reaction
		 *   flag value returned;
		 * - otherwise if there is a parent cooperation than parent coop's
		 *   exception_reaction value returned;
		 * - otherwise SO Environment's exception_reaction is returned.
		 */
		exception_reaction_t
		exception_reaction() const;
		/*!
		 * \}
		 */

		/*!
		 * \since v.5.3.0
		 * \brief Start definition of ad-hoc agent with default
		 * dispatcher binding.
		 *
		 * Usage sample:
		 \code
		 coop->define_agent()
		 		// Set initial agent action.
				.on_start( []() { std::cout << "Hello!" << std::endl; }
				// Set last agent action.
				.on_finish( []() { std::cout << "Bye!" << std::endl; }
				// Subscribe to message from a mbox.
				.event( mbox, []() { ... } );
				// Subscribe to signal from a mbox.
				.event( mbox, so_5::signal< SIG >, []() { ... } );
		 \endcode
		 */
		inline adhoc_agent_definition_proxy_t
		define_agent()
			{
				auto agent = new adhoc_agent_wrapper_t( m_env );
				this->add_agent( agent );

				return adhoc_agent_definition_proxy_t( agent );
			}

		/*!
		 * \since v.5.3.0
		 * \brief Start definition of ad-hoc agent with the specific
		 * dispatcher binder.
		 *
		 * Usage sample:
		 \code
		 coop->define_agent(
		 		so_5::disp::active_obj::create_disp_binder( "active_obj" ) )
		 		// Set initial agent action.
				.on_start( []() { std::cout << "Hello!" << std::endl; }
				// Set last agent action.
				.on_finish( []() { std::cout << "Bye!" << std::endl; }
				// Subscribe to message from a mbox.
				.event( mbox, []() { ... } );
				// Subscribe to signal from a mbox.
				.event( mbox, so_5::signal< SIG >, []() { ... } );
		 \endcode
		 */
		inline adhoc_agent_definition_proxy_t
		define_agent(
			//! A binder to the dispatcher.
			disp_binder_unique_ptr_t binder )
			{
				auto agent = new adhoc_agent_wrapper_t( m_env );
				this->add_agent( agent, std::move(binder) );

				return adhoc_agent_definition_proxy_t( agent );
			}

		/*!
		 * \since v.5.3.0
		 * \brief Access to SO Environment for which cooperation is bound.
		 */
		inline environment_t &
		environment() const
			{
				return m_env;
			}

	private:
		//! Information about agent and its dispatcher binding.
		struct agent_with_disp_binder_t
		{
			agent_with_disp_binder_t(
				const agent_ref_t & agent_ref,
				const disp_binder_ref_t & binder )
				:
					m_agent_ref( agent_ref ),
					m_binder( binder )
			{}

			//! Agent.
			agent_ref_t m_agent_ref;

			//! Agent to dispatcher binder.
			disp_binder_ref_t m_binder;
		};

		//! Typedef for the agent information container.
		typedef std::vector< agent_with_disp_binder_t > agent_array_t;

		/*!
		 * \since v.5.2.3
		 * \brief Registration status.
		 */
		enum registration_status_t
		{
			//! Cooperation is not registered yet.
			COOP_NOT_REGISTERED,
			//! Cooperation is registered.
			/*!
			 * Reference count for cooperation in that state should
			 * be greater than zero.
			 */
			COOP_REGISTERED,
			//! Cooperation is in deregistration process.
			/*!
			 * Reference count for cooperation in that state should
			 * be zero.
			 */
			COOP_DEREGISTERING
		};

		/*!
		 * \since v.5.2.3
		 * \brief Type of user resource deleter.
		 */
		typedef std::function< void() > resource_deleter_t;

		/*!
		 * \since v.5.2.3
		 * \brief Type of container for user resource deleters.
		 */
		typedef std::vector< resource_deleter_t > resource_deleter_vector_t;

		//! Cooperation name.
		const std::string m_coop_name;

		//! Default agent to the dispatcher binder.
		disp_binder_ref_t m_coop_disp_binder;

		//! Cooperation agents.
		agent_array_t m_agent_array;

		//! SObjectizer Environment for which cooperation is created.
		environment_t & m_env;

		//! Count for entities.
		/*!
		 * Since v.5.2.3 this counter includes:
		 * - count of agents from cooperation;
		 * - count of direct child cooperations;
		 * - usage of cooperation pointer in cooperation registration routine.
		 *
		 * \sa agent_coop_t::increment_usage_count()
		 */
		atomic_counter_t m_reference_count;

		/*!
		 * \since v.5.2.3.
		 * \brief Name of the parent cooperation.
		 *
		 * Empty value means than there is no parent cooperation.
		 */
		std::string m_parent_coop_name;

		/*!
		 * \since v.5.2.3.
		 * \brief Pointer to parent cooperation.
		 *
		 * Gets the value only if there is the parent cooperation and
		 * cooperation itself is registered successfully.
		 */
		agent_coop_t * m_parent_coop_ptr;

		/*!
		 * \since v.5.2.3
		 * \brief Notificators for registration event.
		 */
		coop_reg_notificators_container_ref_t m_reg_notificators;

		/*!
		 * \since v.5.2.3
		 * \brief Notificators for deregistration event.
		 */
		coop_dereg_notificators_container_ref_t m_dereg_notificators;

		/*!
		 * \since v.5.2.3
		 * \brief The registration status of cooperation.
		 *
		 * By default cooperation has NOT_REGISTERED status.
		 * It is changed to REGISTERED after successfull completion
		 * of all registration-specific actions.
		 *
		 * And then changed to DEREGISTERING when m_reference_count
		 * becames zero and final deregistration demand would be
		 * put to deregistration thread.
		 */
		registration_status_t m_registration_status;

		/*!
		 * \since v.5.2.3
		 * \brief Container of user resource deleters.
		 */
		resource_deleter_vector_t m_resource_deleters;

		/*!
		 * \since v.5.2.3
		 * \brief Deregistration reason.
		 *
		 * Receives actual value only in do_deregistration_specific_actions().
		 */
		coop_dereg_reason_t m_dereg_reason;

		/*!
		 * \since v.5.3.0
		 * \brief A reaction to non-handled exception.
		 *
		 * By default inherit_exception_reaction is used. It means that
		 * actual exception reaction should be provided by parent coop
		 * or by SO Environment.
		 */
		exception_reaction_t m_exception_reaction;

		//! Add agent to cooperation.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		void
		do_add_agent(
			const agent_ref_t & agent_ref );

		//! Add agent to the cooperation with the dispatcher binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		void
		do_add_agent(
			//! Agent.
			const agent_ref_t & agent_ref,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder );

		/*!
		 * \since v.5.2.3
		 * \brief Perform all neccessary actions related to
		 * cooperation registration.
		 */
		void
		do_registration_specific_actions(
			//! Pointer to the parent cooperation.
			//! Contains nullptr if there is no parent cooperation.
			agent_coop_t * agent_coop );

		/*!
		 * \since v.5.2.3
		 * \brief Perform all necessary actions related to
		 * cooperation deregistration.
		 */
		void
		do_deregistration_specific_actions(
			//! Deregistration reason.
			coop_dereg_reason_t dereg_reason );

		//! Bind agents to the cooperation.
		void
		bind_agents_to_coop();

		//! Calls define_agent method for all cooperation agents.
		void
		define_all_agents();

		//! Bind agents to the dispatcher.
		void
		bind_agents_to_disp();

		//! Unbind agent from the dispatcher.
		/*!
		 * Unbinds all agents in range [m_agent_array.begin(), it).
		 */
		void
		unbind_agents_from_disp(
			//! Right border of the processing range.
			agent_array_t::iterator it );

		/*!
		 * \since v.5.2.3
		 * \brief Shutdown all agents as a part of cooperation deregistration.
		 *
		 * An exception from agent_t::shutdown_agent() leads to call to abort().
		 */
		void
		shutdown_all_agents();

		/*!
		 * \since v.5.2.3
		 * \brief Increment usage counter for this cooperation.
		 *
		 * In v.5.2.3 the counter m_reference_count is used to
		 * reflect count of references to the cooperation. There are
		 * the following entities who can refer to cooperation:
		 * - agents from that cooperation. When cooperation is successfully
		 *   registered the counter is incremented by count of agents.
		 *   During cooperation deregistration agents finish their work and
		 *   each agent decrement cooperation usage counter;
		 * - children cooperations. Each child cooperation increments
		 *   reference counter on its registration and decrements counter
		 *   on its deregistration;
		 * - cooperation registration routine. It increment reference counter
		 *   to prevent cooperation deregistration before the end of
		 *   registration process. It is possible if cooperation do its
		 *   work very quickly and initiates deregistration. When cooperation
		 *   has coop_notificators its registration process may be longer
		 *   then cooperation working time. And cooperation could be
		 *   deregistered and even destroyed before return from registration
		 *   routine. To prevent this cooperation registration routine
		 *   increments cooperation usage counter and the begin of process
		 *   and decrement it when registration process finished.
		 */
		void
		increment_usage_count();

		//! Process signal about finished work of an agent or
		//! child cooperation.
		/*!
		 * Cooperation deregistration is a long process. All agents
		 * process events out of their queues. When an agent detects that
		 * no more events in its queue it informs the cooperation about this.
		 *
		 * When cooperation detects that all agents have finished their
		 * work it initiates the agent's destruction.
		 *
		 * Since v.5.2.3 this method used not only for agents of cooperation
		 * but and for children cooperations. Because final step of
		 * cooperation deregistration could be initiated only when all
		 * children cooperations are deregistered and destroyed.
		 */
		void
		decrement_usage_count();

		//! Do the final deregistration stage.
		void
		final_deregister_coop();

		/*!
		 * \since v.5.2.3
		 * \brief Get pointer to the parent cooperation.
		 *
		 * \retval nullptr if there is no parent cooperation.
		 */
		agent_coop_t *
		parent_coop_ptr() const;

		/*!
		 * \since v.5.2.3
		 * \brief Get registration notificators.
		 */
		coop_reg_notificators_container_ref_t
		reg_notificators() const;

		/*!
		 * \since v.5.2.3
		 * \brief Get deregistration notificators.
		 */
		coop_dereg_notificators_container_ref_t
		dereg_notificators() const;

		/*!
		 * \since v.5.2.3
		 * \brief Delete all user resources.
		 */
		void
		delete_user_resources();

		/*!
		 * \since v.5.2.3
		 * \brief Get deregistration reason.
		 */
		const coop_dereg_reason_t &
		dereg_reason() const;
};

/*!
 * \since v.5.2.3
 * \brief A custom deleter for cooperation.
 */
class agent_coop_deleter_t
{
	public :
		inline void
		operator()( agent_coop_t * coop ) { agent_coop_t::destroy( coop ); }
};

//! Typedef for the agent_coop autopointer.
typedef std::unique_ptr< agent_coop_t, agent_coop_deleter_t >
	agent_coop_unique_ptr_t;

//! Typedef for the agent_coop smart pointer.
typedef std::shared_ptr< agent_coop_t > agent_coop_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif